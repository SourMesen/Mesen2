#include "Common.h"
#include "Renderer.h"
#include "DirectXTK/SpriteBatch.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/Video/VideoDecoder.h"
#include "Core/Shared/Video/VideoRenderer.h"
#include "Core/Shared/MessageManager.h"
#include "Core/Shared/SettingTypes.h"
#include "Core/Shared/EmuSettings.h"
#include "Utilities/UTF8Util.h"

using namespace DirectX;

Renderer::Renderer(Emulator* emu, HWND hWnd)
{
	_emu = emu;
	_hWnd = hWnd;

	SetScreenSize(256, 224);
}

Renderer::~Renderer()
{
	VideoRenderer* videoRenderer = _emu->GetVideoRenderer();
	if(videoRenderer) {
		videoRenderer->UnregisterRenderingDevice(this);
	}
	CleanupDevice();
}

void Renderer::SetExclusiveFullscreenMode(bool fullscreen, void* windowHandle)
{
	if(fullscreen != _fullscreen || _hWnd != (HWND)windowHandle) {
		int counter = _resetCounter;

		_hWnd = (HWND)windowHandle;
		_monitorWidth = _emu->GetSettings()->GetVideoConfig().FullscreenResWidth;
		_monitorHeight = _emu->GetSettings()->GetVideoConfig().FullscreenResHeight;

		_newFullscreen = fullscreen;

		while(_resetCounter <= counter) {
			std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(10));
		}
	}
}

DXGI_FORMAT Renderer::GetTextureFormat()
{
	return _useSrgbTextureFormat ? DXGI_FORMAT_B8G8R8A8_UNORM_SRGB : DXGI_FORMAT_B8G8R8A8_UNORM;
}

void Renderer::SetScreenSize(uint32_t width, uint32_t height)
{
	VideoConfig cfg = _emu->GetSettings()->GetVideoConfig();
	FrameInfo rendererSize = _emu->GetVideoRenderer()->GetRendererSize();
	uint32_t refreshRate = _emu->GetFps() < 55 ? cfg.ExclusiveFullscreenRefreshRatePal : cfg.ExclusiveFullscreenRefreshRateNtsc;

	auto needUpdate = [=] {
		return (
			_emuFrameHeight != height ||
			_emuFrameWidth != width ||
			_screenHeight != rendererSize.Height ||
			_screenWidth != rendererSize.Width ||
			_newFullscreen != _fullscreen ||
			_useSrgbTextureFormat != cfg.UseSrgbTextureFormat ||
			(_fullscreen && _fullscreenRefreshRate != refreshRate) ||
			(_fullscreen && (_realScreenHeight != _monitorHeight || _realScreenWidth != _monitorWidth))
		);
	};

	if(needUpdate()) {
		auto frameLock = _frameLock.AcquireSafe();
		auto textureLock = _textureLock.AcquireSafe();
		if(needUpdate()) {
			_emuFrameHeight = height;
			_emuFrameWidth = width;

			bool needReset = _fullscreen != _newFullscreen;
			bool fullscreenResizeMode = _fullscreen && _newFullscreen;

			if(_pSwapChain && _fullscreen && !_newFullscreen) {
				HRESULT hr = _pSwapChain->SetFullscreenState(FALSE, NULL);
				if(FAILED(hr)) {
					MessageManager::Log("SetFullscreenState(FALSE) failed - Error:" + std::to_string(hr));
				}
			}
			
			if(_useSrgbTextureFormat != cfg.UseSrgbTextureFormat) {
				_useSrgbTextureFormat = cfg.UseSrgbTextureFormat;
				needReset = true;
			}

			_fullscreen = _newFullscreen;
			if(_fullscreenRefreshRate != refreshRate) {
				_fullscreenRefreshRate = refreshRate;
				needReset = true;
			}

			_screenHeight = rendererSize.Height;
			_screenWidth = rendererSize.Width;

			if(_fullscreen) {
				if(_realScreenHeight != _monitorHeight) {
					_realScreenHeight = _monitorHeight;
					needReset = true;
				}

				if(_realScreenWidth != _monitorWidth) {
					_realScreenWidth = _monitorWidth;
					needReset = true;
				}

				//Ensure the screen width/height is smaller or equal to the fullscreen resolution, no matter the requested scale
				if(_monitorHeight < _screenHeight || _monitorWidth < _screenWidth) {
					double scale = (double)width / (double)height;
					_screenHeight = _monitorHeight;
					_screenWidth = (uint32_t)(scale * _screenHeight);
					if(_monitorWidth < _screenWidth) {
						_screenWidth = _monitorWidth;
						_screenHeight = (uint32_t)(_screenWidth / scale);
					}
				}
			} else {
				_realScreenHeight = _screenHeight;
				_realScreenWidth = _screenWidth;
			}

			_leftMargin = (_realScreenWidth - _screenWidth) / 2;
			_topMargin = (_realScreenHeight - _screenHeight) / 2;

			_screenBufferSize = _realScreenHeight*_realScreenWidth;

			if(!_pSwapChain || needReset) {
				Reset();
			} else {
				if(fullscreenResizeMode) {
					ResetTextureBuffers();
					CreateEmuTextureBuffers();
				} else {
					ResetTextureBuffers();
					ReleaseRenderTargetView();
					_pSwapChain->ResizeBuffers(1, _realScreenWidth, _realScreenHeight, GetTextureFormat(), 0);
					CreateRenderTargetView();
					CreateEmuTextureBuffers();
				}
			}
		}
	}
}

void Renderer::Reset()
{
	auto lock = _frameLock.AcquireSafe();
	CleanupDevice();
	if(FAILED(InitDevice())) {
		CleanupDevice();
	} else {
		_emu->GetVideoRenderer()->RegisterRenderingDevice(this);
	}

	_resetCounter++;
}

void Renderer::CleanupDevice()
{
	ResetTextureBuffers();
	ReleaseRenderTargetView();
	if(_pSwapChain) {
		_pSwapChain->SetFullscreenState(false, nullptr);
		_pSwapChain->Release();
		_pSwapChain = nullptr;
	}
	if(_pDeviceContext) {
		_pDeviceContext->Release();
		_pDeviceContext = nullptr;
	}
	if(_pd3dDevice) {
		_pd3dDevice->Release();
		_pd3dDevice = nullptr;
	}
	if(_emuHud.Texture) {
		_emuHud.Texture->Release();
		_emuHud.Texture = nullptr;
	}
	if(_emuHud.Shader) {
		_emuHud.Shader->Release();
		_emuHud.Shader = nullptr;
	}
	if(_scriptHud.Texture) {
		_scriptHud.Texture->Release();
		_scriptHud.Texture = nullptr;
	}
	if(_scriptHud.Shader) {
		_scriptHud.Shader->Release();
		_scriptHud.Shader = nullptr;
	}
}

void Renderer::ResetTextureBuffers()
{
	if(_pTexture) {
		_pTexture->Release();
		_pTexture = nullptr;
	}
	if(_pTextureSrv) {
		_pTextureSrv->Release();
		_pTextureSrv = nullptr;
	}

	delete[] _textureBuffer[0];
	_textureBuffer[0] = nullptr;
	delete[] _textureBuffer[1];
	_textureBuffer[1] = nullptr;
}

void Renderer::ReleaseRenderTargetView()
{
	if(_pRenderTargetView) {
		_pRenderTargetView->Release();
		_pRenderTargetView = nullptr;
	}
}

HRESULT Renderer::CreateRenderTargetView()
{
	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	HRESULT hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if(FAILED(hr)) {
		MessageManager::Log("SwapChain::GetBuffer() failed - Error:" + std::to_string(hr));
		return hr;
	}

	hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
	pBackBuffer->Release();
	if(FAILED(hr)) {
		MessageManager::Log("D3DDevice::CreateRenderTargetView() failed - Error:" + std::to_string(hr));
		return hr;
	}

	_pDeviceContext->OMSetRenderTargets(1, &_pRenderTargetView, nullptr);

	return S_OK;
}

HRESULT Renderer::CreateEmuTextureBuffers()
{
	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)_realScreenWidth;
	vp.Height = (FLOAT)_realScreenHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	_pDeviceContext->RSSetViewports(1, &vp);

	_textureBuffer[0] = new uint8_t[_emuFrameWidth*_emuFrameHeight * 4];
	_textureBuffer[1] = new uint8_t[_emuFrameWidth*_emuFrameHeight * 4];
	memset(_textureBuffer[0], 0, _emuFrameWidth*_emuFrameHeight * 4);
	memset(_textureBuffer[1], 0, _emuFrameWidth*_emuFrameHeight * 4);

	_pTexture = CreateTexture(_emuFrameWidth, _emuFrameHeight);
	if(!_pTexture) {
		return S_FALSE;
	}
	_pTextureSrv = GetShaderResourceView(_pTexture);
	if(!_pTextureSrv) {
		return S_FALSE;
	}

	////////////////////////////////////////////////////////////////////////////
	_spriteBatch.reset(new SpriteBatch(_pDeviceContext));

	return S_OK;
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT Renderer::InitDevice()
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = _realScreenWidth;
	sd.BufferDesc.Height = _realScreenHeight;
	sd.BufferDesc.Format = GetTextureFormat();
	sd.BufferDesc.RefreshRate.Numerator = _fullscreenRefreshRate;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.Flags = _fullscreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;
	sd.OutputWindow = _hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
	for(UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
		driverType = driverTypes[driverTypeIndex];
		featureLevel = D3D_FEATURE_LEVEL_11_1;
		hr = D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels, D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &featureLevel, &_pDeviceContext);

		/*if(FAILED(hr)) {
			MessageManager::Log("D3D11CreateDeviceAndSwapChain() failed - Error:" + std::to_string(hr));
		}*/

		if(hr == E_INVALIDARG) {
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			featureLevel = D3D_FEATURE_LEVEL_11_0;
			hr = D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1, D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &featureLevel, &_pDeviceContext);
		}

		if(SUCCEEDED(hr)) {
			break;
		}
	}
		
	if(FAILED(hr)) {
		MessageManager::Log("D3D11CreateDeviceAndSwapChain() failed - Error:" + std::to_string(hr));
		return hr;
	}

	if(_fullscreen) {
		hr = _pSwapChain->SetFullscreenState(TRUE, NULL);
		if(FAILED(hr)) {
			MessageManager::Log("SetFullscreenState(true) failed - Error:" + std::to_string(hr));
			MessageManager::Log("Switching back to windowed mode");
			hr = _pSwapChain->SetFullscreenState(FALSE, NULL);
			if(FAILED(hr)) {
				MessageManager::Log("SetFullscreenState(false) failed - Error:" + std::to_string(hr));
				return hr;
			}
		} else {
			//Get actual monitor resolution (which might differ from the one that was requested)
			HMONITOR monitor = MonitorFromWindow(_hWnd, MONITOR_DEFAULTTOPRIMARY);
			MONITORINFO info = {};
			info.cbSize = sizeof(MONITORINFO);
			GetMonitorInfo(monitor, &info);

			uint32_t monitorWidth = info.rcMonitor.right - info.rcMonitor.left;
			uint32_t monitorHeight = info.rcMonitor.bottom - info.rcMonitor.top;

			if(_monitorHeight != monitorHeight || _monitorWidth != monitorWidth) {
				MessageManager::Log(
					"Requested resolution (" + std::to_string(_monitorWidth) + "x" + std::to_string(_monitorHeight) 
					+ ") is not available. Resetting to nearest match instead: " +
					std::to_string(monitorWidth) + "x" + std::to_string(monitorHeight)
				);
				_monitorWidth = monitorWidth;
				_monitorHeight = monitorHeight;

				//Make UI wait until this 2nd reset is over
				_resetCounter--;
			}
		}
	}

	hr = CreateRenderTargetView();
	if(FAILED(hr)) {
		return hr;
	}
	hr = CreateEmuTextureBuffers();
	if(FAILED(hr)) {
		return hr;
	}

	return S_OK;
}

ID3D11Texture2D* Renderer::CreateTexture(uint32_t width, uint32_t height)
{
	ID3D11Texture2D* texture;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Format = GetTextureFormat();
	desc.MipLevels = 1;
	desc.MiscFlags = 0;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.Width = width;
	desc.Height = height;
	desc.MiscFlags = 0;

	HRESULT hr = _pd3dDevice->CreateTexture2D(&desc, nullptr, &texture);
	if(FAILED(hr)) {
		MessageManager::Log("D3DDevice::CreateTexture() failed - Error:" + std::to_string(hr));
		return nullptr;
	}
	return texture;
}

ID3D11ShaderResourceView* Renderer::GetShaderResourceView(ID3D11Texture2D* texture)
{
	ID3D11ShaderResourceView *shaderResourceView = nullptr;
	HRESULT hr = _pd3dDevice->CreateShaderResourceView(texture, nullptr, &shaderResourceView);
	if(FAILED(hr)) {
		MessageManager::Log("D3DDevice::CreateShaderResourceView() failed - Error:" + std::to_string(hr));
		return nullptr;
	}

	return shaderResourceView;
}

void Renderer::ClearFrame()
{
	//Clear current output and display black frame
	auto lock = _textureLock.AcquireSafe();
	if(_textureBuffer[0]) {
		//_textureBuffer[0] may be null if directx failed to initialize properly
		memset(_textureBuffer[0], 0, _emuFrameWidth * _emuFrameHeight * sizeof(uint32_t));
		_needFlip = true;
		_frameChanged = true;
	}
}

void Renderer::UpdateFrame(RenderedFrame& frame)
{
	SetScreenSize(frame.Width, frame.Height);

	auto lock = _textureLock.AcquireSafe();
	if(_textureBuffer[0]) {
		//_textureBuffer[0] may be null if directx failed to initialize properly
		memcpy(_textureBuffer[0], frame.FrameBuffer, frame.Width*frame.Height*sizeof(uint32_t));
		_needFlip = true;
		_frameChanged = true;
	}
}

void Renderer::DrawScreen()
{
	//Swap buffers - emulator always writes to _textureBuffer[0], screen always draws _textureBuffer[1]
	if(_needFlip) {
		auto lock = _textureLock.AcquireSafe();
		uint8_t* textureBuffer = _textureBuffer[0];
		_textureBuffer[0] = _textureBuffer[1];
		_textureBuffer[1] = textureBuffer;
		_needFlip = false;

		if(_frameChanged) {
			_frameChanged = false;
		}
	}

	//Copy buffer to texture
	uint32_t bpp = 4;
	uint32_t rowPitch = _emuFrameWidth * bpp;
	D3D11_MAPPED_SUBRESOURCE dd;
	HRESULT hr = _pDeviceContext->Map(_pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &dd);
	if(FAILED(hr)) {
		MessageManager::Log("DeviceContext::Map() failed - Error:" + std::to_string(hr));
		return;
	}
	uint8_t* surfacePointer = (uint8_t*)dd.pData;
	uint8_t* videoBuffer = _textureBuffer[1];
	if(rowPitch != dd.RowPitch) {
		for(uint32_t i = 0, iMax = _emuFrameHeight; i < iMax; i++) {
			memcpy(surfacePointer, videoBuffer, rowPitch);
			videoBuffer += rowPitch;
			surfacePointer += dd.RowPitch;
		}
	} else {
		memcpy(surfacePointer, videoBuffer, rowPitch * _emuFrameHeight);
	}
	_pDeviceContext->Unmap(_pTexture, 0);

	RECT destRect;
	destRect.left = _leftMargin;
	destRect.top = _topMargin;
	destRect.right = _screenWidth+_leftMargin;
	destRect.bottom = _screenHeight+_topMargin;

	_spriteBatch->Draw(_pTextureSrv, destRect);
}

bool Renderer::CreateHudTexture(HudRenderInfo& hud, uint32_t newWidth, uint32_t newHeight)
{
	if(hud.Texture) {
		hud.Texture->Release();
		hud.Texture = nullptr;
	}
	if(hud.Shader) {
		hud.Shader->Release();
		hud.Shader = nullptr;
	}

	hud.Width = newWidth;
	hud.Height = newHeight;

	hud.Texture = CreateTexture(hud.Width, hud.Height);
	if(!hud.Texture) {
		return false;
	}
	hud.Shader = GetShaderResourceView(hud.Texture);
	if(!hud.Shader) {
		return false;
	}

	return true;
}

void Renderer::DrawHud(HudRenderInfo& hud, RenderSurfaceInfo& hudSurface)
{
	uint32_t* hudBuffer = hudSurface.Buffer;
	uint32_t newWidth = hudSurface.Width;
	uint32_t newHeight = hudSurface.Height;

	if(newWidth == 0 && newHeight == 0) {
		return;
	}

	bool needRedraw = hudSurface.IsDirty;
	if(hud.Width != newWidth || hud.Height != newHeight || !hud.Texture || !hud.Shader) {
		needRedraw = true;
		if(!CreateHudTexture(hud, newWidth, newHeight)) {
			return;
		}
	}

	if(needRedraw) {
		//Copy buffer to texture
		uint32_t rowPitch = hud.Width * sizeof(uint32_t);
		D3D11_MAPPED_SUBRESOURCE dd;
		HRESULT hr = _pDeviceContext->Map(hud.Texture, 0, D3D11_MAP_WRITE_DISCARD, 0, &dd);
		if(FAILED(hr)) {
			MessageManager::Log("DeviceContext::Map() failed - Error:" + std::to_string(hr));
			return;
		}
		uint8_t* surfacePointer = (uint8_t*)dd.pData;
		uint8_t* videoBuffer = (uint8_t*)hudBuffer;
		if(rowPitch != dd.RowPitch) {
			for(uint32_t i = 0, iMax = hud.Height; i < iMax; i++) {
				memcpy(surfacePointer, videoBuffer, rowPitch);
				videoBuffer += rowPitch;
				surfacePointer += dd.RowPitch;
			}
		} else {
			memcpy(surfacePointer, videoBuffer, hud.Height * rowPitch);
		}
		_pDeviceContext->Unmap(hud.Texture, 0);
	}
	
	RECT destRect;
	destRect.left = _leftMargin;
	destRect.top = _topMargin;
	destRect.right = _screenWidth + _leftMargin;
	destRect.bottom = _screenHeight + _topMargin;

	_spriteBatch->Draw(hud.Shader, destRect);
}

void Renderer::Render(RenderSurfaceInfo& emuHud, RenderSurfaceInfo& scriptHud)
{
	auto lock = _frameLock.AcquireSafe();
	if(_newFullscreen != _fullscreen) {
		SetScreenSize(_emuFrameWidth, _emuFrameHeight);
	}

	if(_pDeviceContext == nullptr) {
		//DirectX failed to initialize, try to init
		Reset();
		if(_pDeviceContext == nullptr) {
			//Can't init, prevent crash
			return;
		}
	}

	VideoConfig cfg = _emu->GetSettings()->GetVideoConfig();

	// Clear the back buffer 
	_pDeviceContext->ClearRenderTargetView(_pRenderTargetView, Colors::Black);

	//Draw screen
	_spriteBatch->Begin(SpriteSortMode_Immediate, cfg.UseBilinearInterpolation);
	DrawScreen();
	_spriteBatch->End();

	//Draw HUD
	_spriteBatch->Begin(SpriteSortMode_Immediate, false);
	DrawHud(_scriptHud, scriptHud);
	DrawHud(_emuHud, emuHud);
	_spriteBatch->End();

	// Present the information rendered to the back buffer to the front buffer (the screen)
	HRESULT hr = _pSwapChain->Present(cfg.VerticalSync ? 1 : 0, 0);
	if(FAILED(hr)) {
		MessageManager::Log("SwapChain::Present() failed - Error:" + std::to_string(hr));
		if(hr == DXGI_ERROR_DEVICE_REMOVED) {
			MessageManager::Log("D3DDevice: GetDeviceRemovedReason: " + std::to_string(_pd3dDevice->GetDeviceRemovedReason()));
		}
		MessageManager::Log("Trying to reset DX...");
		Reset();
	}
}
