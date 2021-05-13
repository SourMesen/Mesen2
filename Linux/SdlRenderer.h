#pragma once
#include <SDL2/SDL.h>
#include "Core/Shared/Interfaces/IRenderingDevice.h"
#include "Utilities/SimpleLock.h"
#include "Core/Shared/Video/VideoRenderer.h"
#include "Core/Shared/Video/BaseRenderer.h"

class Emulator;

class SdlRenderer : public IRenderingDevice, public BaseRenderer
{
private:
	void* _windowHandle;
	SDL_Window* _sdlWindow = nullptr;
	SDL_Renderer *_sdlRenderer = nullptr;
	SDL_Texture *_sdlTexture = nullptr;
	
	bool _useBilinearInterpolation = false;

	static SimpleLock _frameLock;
	uint32_t* _frameBuffer = nullptr;

	const uint32_t _bytesPerPixel = 4;
	uint32_t _screenBufferSize = 0;

	bool _frameChanged = true;

	uint32_t _requiredHeight = 0;
	uint32_t _requiredWidth = 0;

	uint32_t _nesFrameHeight = 0;
	uint32_t _nesFrameWidth = 0;
	uint32_t _newFrameBufferSize = 0;

	bool _vsyncEnabled = false;

	bool Init();
	void Cleanup();
	void LogSdlError(const char* msg);
	void SetScreenSize(uint32_t width, uint32_t height);

public:
	SdlRenderer(shared_ptr<Emulator> emu, void* windowHandle, bool registerAsMessageManager);
	virtual ~SdlRenderer();

	void UpdateFrame(void *frameBuffer, uint32_t width, uint32_t height) override;
	void Render() override;
	void Reset() override;

	void SetFullscreenMode(bool fullscreen, void* windowHandle, uint32_t monitorWidth, uint32_t monitorHeight) override;
};
