#pragma once

#include "Common.h"
#include "Core/Shared/Interfaces/IRenderingDevice.h"
#include "Core/Shared/Interfaces/IMessageManager.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/Timer.h"

using namespace DirectX;

class Emulator;

namespace DirectX {
	class SpriteBatch;
}

struct HudRenderInfo
{
	ID3D11Texture2D* Texture = nullptr;
	ID3D11ShaderResourceView* Shader = nullptr;
	uint32_t Width = 0;
	uint32_t Height = 0;
};

class Renderer final : public IRenderingDevice
{
private:
	Emulator* _emu;

	HWND _hWnd = nullptr;

	ID3D11Device* _pd3dDevice = nullptr;
	ID3D11DeviceContext* _pDeviceContext = nullptr;
	IDXGISwapChain* _pSwapChain = nullptr;
	ID3D11RenderTargetView* _pRenderTargetView = nullptr;

	atomic<bool> _needFlip = false;
	uint8_t* _textureBuffer[2] = { nullptr, nullptr };
	ID3D11Texture2D* _pTexture = nullptr;
	ID3D11ShaderResourceView* _pTextureSrv = nullptr;

	HudRenderInfo _emuHud = {};
	HudRenderInfo _scriptHud = {};

	bool _frameChanged = true;
	SimpleLock _frameLock;
	SimpleLock _textureLock;

	unique_ptr<SpriteBatch> _spriteBatch;

	const uint32_t _bytesPerPixel = 4;
	uint32_t _screenBufferSize = 0;

	bool _newFullscreen = false;
	bool _fullscreen = false;
	uint32_t _fullscreenRefreshRate = 60;
	bool _useSrgbTextureFormat = false;

	uint32_t _screenWidth = 0;
	uint32_t _screenHeight = 0;

	uint32_t _realScreenHeight = 240;
	uint32_t _realScreenWidth = 256;
	uint32_t _leftMargin = 0;
	uint32_t _topMargin = 0;
	uint32_t _monitorWidth = 0;
	uint32_t _monitorHeight = 0;

	uint32_t _emuFrameHeight = 0;
	uint32_t _emuFrameWidth = 0;

	atomic<int> _resetCounter = 0;

	HRESULT InitDevice();
	void CleanupDevice();

	void SetScreenSize(uint32_t width, uint32_t height);

	ID3D11Texture2D* CreateTexture(uint32_t width, uint32_t height);
	ID3D11ShaderResourceView* GetShaderResourceView(ID3D11Texture2D* texture);
	void DrawScreen();

	bool CreateHudTexture(HudRenderInfo& hud, uint32_t newWidth, uint32_t newHeight);
	void DrawHud(HudRenderInfo& hud, RenderSurfaceInfo& hudSurface);
		
	HRESULT CreateRenderTargetView();
	void ReleaseRenderTargetView();
	HRESULT CreateEmuTextureBuffers();
	void ResetTextureBuffers();
	
	DXGI_FORMAT GetTextureFormat();

public:
	Renderer(Emulator* emu, HWND hWnd);
	~Renderer();

	void SetExclusiveFullscreenMode(bool fullscreen, void* windowHandle) override;

	void Reset() override;
	void Render(RenderSurfaceInfo& emuHud, RenderSurfaceInfo& scriptHud) override;
	void ClearFrame() override;

	void UpdateFrame(RenderedFrame& frame) override;
};