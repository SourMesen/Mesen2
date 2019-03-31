#include "SdlRenderer.h"
#include "../Core/Console.h"
#include "../Core/Debugger.h"
#include "../Core/VideoRenderer.h"
#include "../Core/VideoDecoder.h"
#include "../Core/EmuSettings.h"
#include "../Core/MessageManager.h"

SimpleLock SdlRenderer::_reinitLock;
SimpleLock SdlRenderer::_frameLock;

SdlRenderer::SdlRenderer(shared_ptr<Console> console, void* windowHandle, bool registerAsMessageManager) : BaseRenderer(console, registerAsMessageManager), _windowHandle(windowHandle)
{
	_frameBuffer = nullptr;
	SetScreenSize(512,480);
}

SdlRenderer::~SdlRenderer()
{
	shared_ptr<VideoRenderer> videoRenderer = _console->GetVideoRenderer();
	if(videoRenderer) {
		videoRenderer->UnregisterRenderingDevice(this);
	}
	Cleanup();
}

void SdlRenderer::SetFullscreenMode(bool fullscreen, void* windowHandle, uint32_t monitorWidth, uint32_t monitorHeight)
{
	//TODO: Implement exclusive fullscreen for Linux
}

bool SdlRenderer::Init()
{
	auto log = [](const char* msg) {
		MessageManager::Log(msg);
		MessageManager::Log(SDL_GetError());		
	};

	if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		log("[SDL] Failed to initialize video subsystem.");
		return false;
	};

	_sdlWindow = SDL_CreateWindowFrom(_windowHandle);
	if(!_sdlWindow) {
		log("[SDL] Failed to create window from handle.");
		return false;
	}

	//Hack to make this work properly - otherwise SDL_CreateRenderer never returns
	_sdlWindow->flags |= SDL_WINDOW_OPENGL;

	if(SDL_GL_LoadLibrary(NULL) != 0) {
		log("[SDL] Failed to initialize OpenGL, attempting to continue with initialization.");
	}

	uint32_t baseFlags = _vsyncEnabled ? SDL_RENDERER_PRESENTVSYNC : 0;

	_sdlRenderer = SDL_CreateRenderer(_sdlWindow, -1, baseFlags | SDL_RENDERER_ACCELERATED);
	if(!_sdlRenderer) {
		log("[SDL] Failed to create accelerated renderer.");

		MessageManager::Log("[SDL] Attempting to create software renderer...");
		_sdlRenderer = SDL_CreateRenderer(_sdlWindow, -1, baseFlags | SDL_RENDERER_SOFTWARE);
		if(!_sdlRenderer) {
			log("[SDL] Failed to create software renderer.");
			return false;
		}		
	}

	_sdlTexture = SDL_CreateTexture(_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, _nesFrameWidth, _nesFrameHeight);
	if(!_sdlTexture) {
		log("[SDL] Failed to create texture.");
		return false;
	}

	_spriteFont.reset(new SpriteFont(_sdlRenderer, "Resources/Font.24.spritefont"));
	_largeFont.reset(new SpriteFont(_sdlRenderer, "Resources/Font.64.spritefont"));

	SDL_SetWindowSize(_sdlWindow, _screenWidth, _screenHeight);

	_frameBuffer = new uint32_t[_nesFrameHeight*_nesFrameWidth];
	memset(_frameBuffer, 0, _nesFrameHeight*_nesFrameWidth*_bytesPerPixel);

	return true;
}

void SdlRenderer::Cleanup()
{
	if(_sdlTexture) {
		SDL_DestroyTexture(_sdlTexture);
		_sdlTexture = nullptr;		
	}
	if(_sdlRenderer) {
		SDL_DestroyRenderer(_sdlRenderer);
		_sdlRenderer = nullptr;
	}
	if(_frameBuffer) {
		delete[] _frameBuffer; 
		_frameBuffer = nullptr;
	}
}

void SdlRenderer::Reset()
{
	Cleanup();
	if(Init()) {
		_console->GetVideoRenderer()->RegisterRenderingDevice(this);
	} else {
		Cleanup();
	}
}

void SdlRenderer::SetScreenSize(uint32_t width, uint32_t height)
{
	ScreenSize screenSize = _console->GetVideoDecoder()->GetScreenSize(false);
	
	VideoConfig cfg = _console->GetSettings()->GetVideoConfig();
	if(_screenHeight != (uint32_t)screenSize.Height || _screenWidth != (uint32_t)screenSize.Width || _nesFrameHeight != height || _nesFrameWidth != width || _useBilinearInterpolation != cfg.UseBilinearInterpolation || _vsyncEnabled != cfg.VerticalSync) {
		_reinitLock.Acquire();

		_vsyncEnabled = cfg.VerticalSync;
		_useBilinearInterpolation = cfg.UseBilinearInterpolation;

		_nesFrameHeight = height;
		_nesFrameWidth = width;
		_newFrameBufferSize = width*height;

		_screenHeight = screenSize.Height;
		_screenWidth = screenSize.Width;

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, _useBilinearInterpolation ? "1" : "0");
		_screenBufferSize = _screenHeight*_screenWidth;

		Reset();
		_reinitLock.Release();
	}	
}

void SdlRenderer::UpdateFrame(void *frameBuffer, uint32_t width, uint32_t height)
{
	SetScreenSize(width, height);
	_frameLock.Acquire();
	memcpy(_frameBuffer, frameBuffer, width*height*_bytesPerPixel);
	_frameChanged = true;	
	_frameLock.Release();
}

void SdlRenderer::Render()
{
	if(!_sdlRenderer || !_sdlTexture) {
		return;
	}

	bool paused = _console->IsPaused() && _console->IsRunning();

	if(_noUpdateCount > 10 || _frameChanged || paused || IsMessageShown()) {	
		auto lock = _reinitLock.AcquireSafe();

		SDL_RenderClear(_sdlRenderer);

		uint8_t *textureBuffer;
		int rowPitch;
		SDL_LockTexture(_sdlTexture, nullptr, (void**)&textureBuffer, &rowPitch);
		{
			auto frameLock = _frameLock.AcquireSafe();
			uint32_t* ppuFrameBuffer = _frameBuffer;
			for(uint32_t i = 0, iMax = _nesFrameHeight; i < iMax; i++) {
				memcpy(textureBuffer, ppuFrameBuffer, _nesFrameWidth*_bytesPerPixel);
				ppuFrameBuffer += _nesFrameWidth;
				textureBuffer += rowPitch;
			}
		}
		SDL_UnlockTexture(_sdlTexture);

		if(_frameChanged) {
			_renderedFrameCount++;
			_frameChanged = false;
		}

		SDL_Rect source = {0, 0, (int)_nesFrameWidth, (int)_nesFrameHeight };
		SDL_Rect dest = {0, 0, (int)_screenWidth, (int)_screenHeight };
		SDL_RenderCopy(_sdlRenderer, _sdlTexture, &source, &dest);

		if(paused) {
			DrawPauseScreen();
		} else if(_console->GetVideoDecoder()->IsRunning()) {
			DrawCounters();
		}

		DrawToasts();

		SDL_RenderPresent(_sdlRenderer);
	} else {
		_noUpdateCount++;
	}
}

void SdlRenderer::DrawPauseScreen()
{
	DrawString(L"I", 15, 15, 106, 90, 205, 168);
	DrawString(L"I", 23, 15, 106, 90, 205, 168);
}

void SdlRenderer::DrawString(std::wstring message, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t opacity)
{
	const wchar_t *text = message.c_str();
	_spriteFont->DrawString(_sdlRenderer, text, x, y, r, g, b);
}

float SdlRenderer::MeasureString(std::wstring text)
{
	return _spriteFont->MeasureString(text.c_str()).x;
}

bool SdlRenderer::ContainsCharacter(wchar_t character)
{
	return _spriteFont->ContainsCharacter(character);
}