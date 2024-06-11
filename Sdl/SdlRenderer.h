#pragma once
#include "SDL.h"
#include "Core/Shared/Interfaces/IRenderingDevice.h"
#include "Utilities/SimpleLock.h"
#include "Core/Shared/Video/VideoRenderer.h"
#include "Core/Shared/RenderedFrame.h"

class Emulator;

struct HudRenderInfo
{
	SDL_Texture* Texture = nullptr;
	uint32_t Width = 0;
	uint32_t Height = 0;
};

class SdlRenderer : public IRenderingDevice
{
private:
	Emulator* _emu;

	void* _windowHandle;
	SDL_Window* _sdlWindow = nullptr;
	SDL_Renderer *_sdlRenderer = nullptr;
	SDL_Texture* _sdlTexture = nullptr;

	HudRenderInfo _emuHud = {};
	HudRenderInfo _scriptHud = {};
	
	bool _useBilinearInterpolation = false;

	static SimpleLock _frameLock;
	uint32_t* _frameBuffer = nullptr;

	const uint32_t _bytesPerPixel = 4;
	uint32_t _screenBufferSize = 0;

	bool _frameChanged = true;

	uint32_t _screenWidth = 0;
	uint32_t _screenHeight = 0;

	uint32_t _requiredHeight = 0;
	uint32_t _requiredWidth = 0;

	uint32_t _frameHeight = 0;
	uint32_t _frameWidth = 0;
	uint32_t _newFrameBufferSize = 0;

	bool _vsyncEnabled = false;

	bool Init();
	bool InitTexture();
	void Cleanup();
	void LogSdlError(const char* msg);
	void SetScreenSize(uint32_t width, uint32_t height);
	
	bool UpdateHudSize(HudRenderInfo& hud, uint32_t width, uint32_t height);
	void UpdateHudTexture(HudRenderInfo& hud, uint32_t* src);

public:
	SdlRenderer(Emulator* emu, void* windowHandle);
	virtual ~SdlRenderer();

	void ClearFrame() override;
	void UpdateFrame(RenderedFrame& frame) override;
	void Render(RenderSurfaceInfo& emuHud, RenderSurfaceInfo& scriptHud) override;
	void Reset() override;
	void OnRendererThreadStarted() override;

	void SetExclusiveFullscreenMode(bool fullscreen, void* windowHandle) override;
};
