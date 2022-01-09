#include "stdafx.h"
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

Renderer::Renderer(shared_ptr<Emulator> emu, HWND hWnd, bool registerAsMessageManager) : BaseRenderer(emu, registerAsMessageManager)
{
	_hWnd = hWnd;

	SetScreenSize(256, 224);
}

Renderer::~Renderer()
{
	shared_ptr<VideoRenderer> videoRenderer = _emu->GetVideoRenderer();
	if(videoRenderer) {
		videoRenderer->UnregisterRenderingDevice(this);
	}
	CleanupDevice();
}

void Renderer::SetFullscreenMode(bool fullscreen, void* windowHandle, uint32_t monitorWidth, uint32_t monitorHeight)
{
	if(fullscreen != _fullscreen || _hWnd != (HWND)windowHandle) {
		_hWnd = (HWND)windowHandle;
		_monitorWidth = monitorWidth;
		_monitorHeight = monitorHeight;
		_newFullscreen = fullscreen;
	}
}

void Renderer::SetScreenSize(uint32_t width, uint32_t height)
{
	VideoConfig cfg = _emu->GetSettings()->GetVideoConfig();
	FrameInfo rendererSize = _emu->GetVideoRenderer()->GetRendererSize();
	if(_nesFrameHeight != height || _nesFrameWidth != width || _screenHeight != rendererSize.Height || _screenWidth != rendererSize.Width || _newFullscreen != _fullscreen || _useBilinearInterpolation != cfg.UseBilinearInterpolation) {
		auto frameLock = _frameLock.AcquireSafe();
		auto textureLock = _textureLock.AcquireSafe();
		if(_nesFrameHeight != height || _nesFrameWidth != width || _screenHeight != rendererSize.Height || _screenWidth != rendererSize.Width || _newFullscreen != _fullscreen || _useBilinearInterpolation != cfg.UseBilinearInterpolation) {
			_nesFrameHeight = height;
			_nesFrameWidth = width;
			_newFrameBufferSize = width*height;

			bool needReset = _fullscreen != _newFullscreen || _useBilinearInterpolation != cfg.UseBilinearInterpolation;
			bool fullscreenResizeMode = _fullscreen && _newFullscreen;

			if(_pSwapChain && _fullscreen && !_newFullscreen) {
				HRESULT hr = _pSwapChain->SetFullscreenState(FALSE, NULL);
				if(FAILED(hr)) {
					MessageManager::Log("SetFullscreenState(FALSE) failed - Error:" + std::to_string(hr));
				}
			}

			_fullscreen = _newFullscreen;

			_screenHeight = rendererSize.Height;
			_screenWidth = rendererSize.Width;

			if(_fullscreen) {
				_realScreenHeight = _monitorHeight;
				_realScreenWidth = _monitorWidth;

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
					ResetNesBuffers();
					CreateNesBuffers();
				} else {
					ResetNesBuffers();
					ReleaseRenderTargetView();
					_pSwapChain->ResizeBuffers(1, _realScreenWidth, _realScreenHeight, DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, 0);
					CreateRenderTargetView();
					CreateNesBuffers();
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
}

void Renderer::CleanupDevice()
{
	ResetNesBuffers();
	ReleaseRenderTargetView();
	if(_pAlphaEnableBlendingState) {
		_pAlphaEnableBlendingState->Release();
		_pAlphaEnableBlendingState = nullptr;
	}
	if(_pDepthDisabledStencilState) {
		_pDepthDisabledStencilState->Release();
		_pDepthDisabledStencilState = nullptr;
	}
	if(_samplerState) {
		_samplerState->Release();
		_samplerState = nullptr;
	}
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
}

void Renderer::ResetNesBuffers()
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

HRESULT Renderer::CreateNesBuffers()
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

	_textureBuffer[0] = new uint8_t[_nesFrameWidth*_nesFrameHeight * 4];
	_textureBuffer[1] = new uint8_t[_nesFrameWidth*_nesFrameHeight * 4];
	memset(_textureBuffer[0], 0, _nesFrameWidth*_nesFrameHeight * 4);
	memset(_textureBuffer[1], 0, _nesFrameWidth*_nesFrameHeight * 4);

	_pTexture = CreateTexture(_nesFrameWidth, _nesFrameHeight);
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
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
	sd.BufferDesc.RefreshRate.Numerator = _emu->GetSettings()->GetVideoConfig().ExclusiveFullscreenRefreshRate;
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
		}
	}

	hr = CreateRenderTargetView();
	if(FAILED(hr)) {
		return hr;
	}

	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
	ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));
	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabledStencilDesc.StencilEnable = true;
	depthDisabledStencilDesc.StencilReadMask = 0xFF;
	depthDisabledStencilDesc.StencilWriteMask = 0xFF;
	depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the state using the device.
	hr = _pd3dDevice->CreateDepthStencilState(&depthDisabledStencilDesc, &_pDepthDisabledStencilState);
	if(FAILED(hr)) {
		MessageManager::Log("D3DDevice::CreateDepthStencilState() failed - Error:" + std::to_string(hr));
		return hr;
	}

	// Clear the blend state description.
	D3D11_BLEND_DESC blendStateDescription;
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));

	// Create an alpha enabled blend state description.
	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = 0x0f;

	// Create the blend state using the description.
	hr = _pd3dDevice->CreateBlendState(&blendStateDescription, &_pAlphaEnableBlendingState);
	if(FAILED(hr)) {
		MessageManager::Log("D3DDevice::CreateBlendState() failed - Error:" + std::to_string(hr));
		return hr;
	}

	float blendFactor[4];
	blendFactor[0] = 0.0f;
	blendFactor[1] = 0.0f;
	blendFactor[2] = 0.0f;
	blendFactor[3] = 0.0f;
	
	_pDeviceContext->OMSetBlendState(_pAlphaEnableBlendingState, blendFactor, 0xffffffff);
	_pDeviceContext->OMSetDepthStencilState(_pDepthDisabledStencilState, 1);

	hr = CreateNesBuffers();
	if(FAILED(hr)) {
		return hr;
	}

	hr = CreateSamplerState();
	if(FAILED(hr)) {
		return hr;
	}

	return S_OK;
}

HRESULT Renderer::CreateSamplerState()
{
	_useBilinearInterpolation = _emu->GetSettings()->GetVideoConfig().UseBilinearInterpolation;

	//Sample state
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = _useBilinearInterpolation ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	//samplerDesc.BorderColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	HRESULT hr = _pd3dDevice->CreateSamplerState(&samplerDesc, &_samplerState);
	if(FAILED(hr)) {
		MessageManager::Log("D3DDevice::CreateSamplerState() failed - Error:" + std::to_string(hr));
	}

	return hr;
}

ID3D11Texture2D* Renderer::CreateTexture(uint32_t width, uint32_t height)
{
	ID3D11Texture2D* texture;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
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

void Renderer::UpdateFrame(void *frameBuffer, uint32_t width, uint32_t height)
{
	SetScreenSize(width, height);

	uint32_t bpp = 4;
	auto lock = _textureLock.AcquireSafe();
	if(_textureBuffer[0]) {
		//_textureBuffer[0] may be null if directx failed to initialize properly
		memcpy(_textureBuffer[0], frameBuffer, width*height*bpp);
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
	uint32_t rowPitch = _nesFrameWidth * bpp;
	D3D11_MAPPED_SUBRESOURCE dd;
	HRESULT hr = _pDeviceContext->Map(_pTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &dd);
	if(FAILED(hr)) {
		MessageManager::Log("DeviceContext::Map() failed - Error:" + std::to_string(hr));
		return;
	}
	uint8_t* surfacePointer = (uint8_t*)dd.pData;
	uint8_t* videoBuffer = _textureBuffer[1];
	for(uint32_t i = 0, iMax = _nesFrameHeight; i < iMax; i++) {
		memcpy(surfacePointer, videoBuffer, rowPitch);
		videoBuffer += rowPitch;
		surfacePointer += dd.RowPitch;
	}
	_pDeviceContext->Unmap(_pTexture, 0);

	RECT destRect;
	destRect.left = _leftMargin;
	destRect.top = _topMargin;
	destRect.right = _screenWidth+_leftMargin;
	destRect.bottom = _screenHeight+_topMargin;

	_spriteBatch->Draw(_pTextureSrv, destRect);
}

void Renderer::Render()
{
	bool paused = _emu->IsPaused();

	auto lock = _frameLock.AcquireSafe();
	if(_newFullscreen != _fullscreen) {
		SetScreenSize(_nesFrameWidth, _nesFrameHeight);
	}

	if(_pDeviceContext == nullptr) {
		//DirectX failed to initialize, try to init
		Reset();
		if(_pDeviceContext == nullptr) {
			//Can't init, prevent crash
			return;
		}
	}

	// Clear the back buffer 
	_pDeviceContext->ClearRenderTargetView(_pRenderTargetView, Colors::Black);

	_spriteBatch->Begin(SpriteSortMode_Deferred, nullptr, _samplerState);

	//Draw screen
	DrawScreen();

	_spriteBatch->End();

	// Present the information rendered to the back buffer to the front buffer (the screen)

	bool waitVSync = _emu->GetSettings()->GetVideoConfig().VerticalSync;
	HRESULT hr = _pSwapChain->Present(waitVSync ? 1 : 0, 0);
	if(FAILED(hr)) {
		MessageManager::Log("SwapChain::Present() failed - Error:" + std::to_string(hr));
		if(hr == DXGI_ERROR_DEVICE_REMOVED) {
			MessageManager::Log("D3DDevice: GetDeviceRemovedReason: " + std::to_string(_pd3dDevice->GetDeviceRemovedReason()));
		}
		MessageManager::Log("Trying to reset DX...");
		Reset();
	}
}
