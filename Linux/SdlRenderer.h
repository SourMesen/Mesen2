#pragma once
#include <SDL2/SDL.h>
#include "../Core/IRenderingDevice.h"
#include "../Utilities/SimpleLock.h"
#include "../Core/VideoRenderer.h"
#include "../Core/BaseRenderer.h"
#include "SpriteFont.h"

struct SDL_Window
{
	const void *magic;
    Uint32 id;
    char *title;
    SDL_Surface *icon;
    int x, y;
    int w, h;
    int min_w, min_h;
    int max_w, max_h;
    Uint32 flags;
};
typedef struct SDL_Window SDL_Window;

class Console;

class SdlRenderer : public IRenderingDevice, public BaseRenderer
{
private:
	void* _windowHandle;
	SDL_Window* _sdlWindow = nullptr;
	SDL_Renderer *_sdlRenderer = nullptr;
	SDL_Texture *_sdlTexture = nullptr;
	std::unique_ptr<SpriteFont> _spriteFont;
	std::unique_ptr<SpriteFont> _largeFont;
	
	bool _useBilinearInterpolation = false;

	static SimpleLock _frameLock;
	uint32_t* _frameBuffer = nullptr;

	const uint32_t _bytesPerPixel = 4;
	uint32_t _screenBufferSize = 0;

	bool _frameChanged = true;
	uint32_t _noUpdateCount = 0;

	uint32_t _requiredHeight = 0;
	uint32_t _requiredWidth = 0;

	uint32_t _nesFrameHeight = 0;
	uint32_t _nesFrameWidth = 0;
	uint32_t _newFrameBufferSize = 0;

	bool _vsyncEnabled = false;

	bool Init();
	void Cleanup();
	void SetScreenSize(uint32_t width, uint32_t height);

	void DrawPauseScreen();

	float MeasureString(std::wstring text) override;
	bool ContainsCharacter(wchar_t character) override;

public:
	SdlRenderer(shared_ptr<Console> console, void* windowHandle, bool registerAsMessageManager);
	virtual ~SdlRenderer();

	void UpdateFrame(void *frameBuffer, uint32_t width, uint32_t height) override;
	void Render() override;
	void Reset() override;

	void DrawString(std::wstring message, int x, int y, uint8_t r = 255, uint8_t g = 255, uint8_t b = 255, uint8_t opacity = 255) override;
	
	void SetFullscreenMode(bool fullscreen, void* windowHandle, uint32_t monitorWidth, uint32_t monitorHeight) override;
};
