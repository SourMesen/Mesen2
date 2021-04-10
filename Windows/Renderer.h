#pragma once

#include "stdafx.h"
#include "Core/Shared/Interfaces/IRenderingDevice.h"
#include "Core/Shared/Interfaces/IMessageManager.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/Timer.h"
#include "Core/Shared/Video/BaseRenderer.h"

using namespace DirectX;

class Emulator;

namespace DirectX {
	class SpriteBatch;
	class SpriteFont;
}

class Renderer : public BaseRenderer, public IRenderingDevice
{
private:
	HWND                    _hWnd = nullptr;

	ID3D11Device*           _pd3dDevice = nullptr;
	ID3D11DeviceContext*    _pDeviceContext = nullptr;
	IDXGISwapChain*         _pSwapChain = nullptr;
	ID3D11RenderTargetView* _pRenderTargetView = nullptr;
	ID3D11DepthStencilState* _pDepthDisabledStencilState = nullptr;
	ID3D11BlendState*			_pAlphaEnableBlendingState = nullptr;

	ID3D11SamplerState*		_samplerState = nullptr;
		
	atomic<bool>				_needFlip = false;
	uint8_t*						_textureBuffer[2] = { nullptr, nullptr };
	ID3D11Texture2D*			_pTexture = nullptr;
	ID3D11ShaderResourceView*	_pTextureSrv = nullptr;
	ID3D11Texture2D*			_overlayTexture = nullptr;
	ID3D11ShaderResourceView*	_pOverlaySrv = nullptr;

	bool							_frameChanged = true;
	SimpleLock					_frameLock;
	SimpleLock					_textureLock;

	bool _useBilinearInterpolation = false;

	unique_ptr<SpriteFont>	_font;
	unique_ptr<SpriteFont>	_largeFont;
		
	unique_ptr<SpriteBatch> _spriteBatch;

	const uint32_t _bytesPerPixel = 4;
	uint32_t _screenBufferSize = 0;

	bool _newFullscreen = false;
	bool _fullscreen = false;

	uint32_t _realScreenHeight = 240;
	uint32_t _realScreenWidth = 256;
	uint32_t _leftMargin = 0;
	uint32_t _topMargin = 0;
	uint32_t _monitorWidth = 0;
	uint32_t _monitorHeight = 0;

	uint32_t _nesFrameHeight = 0;
	uint32_t _nesFrameWidth = 0;
	uint32_t _newFrameBufferSize = 0;

	uint32_t _noUpdateCount = 0;

	HRESULT InitDevice();
	void CleanupDevice();

	void SetScreenSize(uint32_t width, uint32_t height);

	ID3D11Texture2D* CreateTexture(uint32_t width, uint32_t height);
	ID3D11ShaderResourceView* GetShaderResourceView(ID3D11Texture2D* texture);
	void DrawScreen();
	void DrawPauseScreen();
		
	void DrawString(string message, float x, float y, DirectX::FXMVECTOR color, float scale, SpriteFont* font = nullptr);
	void DrawString(std::wstring message, float x, float y, DirectX::FXMVECTOR color, float scale, SpriteFont* font = nullptr);

	void DrawString(std::wstring message, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t opacity);
	float MeasureString(std::wstring text);
	bool ContainsCharacter(wchar_t character);

	HRESULT CreateRenderTargetView();
	void ReleaseRenderTargetView();
	HRESULT CreateNesBuffers();
	void ResetNesBuffers();
	HRESULT CreateSamplerState();

public:
	Renderer(shared_ptr<Emulator> emu, HWND hWnd, bool registerAsMessageManager);
	~Renderer();

	void SetFullscreenMode(bool fullscreen, void* windowHandle, uint32_t monitorWidth, uint32_t monitorHeight);

	void Reset();
	void Render();

	void UpdateFrame(void *frameBuffer, uint32_t width, uint32_t height);
};