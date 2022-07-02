#include "SdlRenderer.h"
#include "Core/Debugger/Debugger.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/Video/VideoRenderer.h"
#include "Core/Shared/Video/VideoDecoder.h"
#include "Core/Shared/EmuSettings.h"
#include "Core/Shared/MessageManager.h"
#include "Core/Shared/RenderedFrame.h"

SimpleLock SdlRenderer::_frameLock;

SdlRenderer::SdlRenderer(Emulator* emu, void* windowHandle, bool registerAsMessageManager) : BaseRenderer(emu, registerAsMessageManager), _windowHandle(windowHandle)
{
	_frameBuffer = nullptr;
	_requiredWidth = 256;
	_requiredHeight = 240;
	
	_emu->GetVideoRenderer()->RegisterRenderingDevice(this);
}

SdlRenderer::~SdlRenderer()
{
	_emu->GetVideoRenderer()->UnregisterRenderingDevice(this);

	Cleanup();
	delete[] _frameBuffer;	
}

void SdlRenderer::LogSdlError(const char* msg)
{
	MessageManager::Log(msg);
	MessageManager::Log(SDL_GetError());
}

void SdlRenderer::SetExclusiveFullscreenMode(bool fullscreen, void* windowHandle)
{
	//TODO: Implement exclusive fullscreen for Linux
}

bool SdlRenderer::Init()
{
	if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		LogSdlError("[SDL] Failed to initialize video subsystem.");
		return false;
	};

	_sdlWindow = SDL_CreateWindowFrom(_windowHandle);
	if(!_sdlWindow) {
		LogSdlError("[SDL] Failed to create window from handle.");
		return false;
	}

	if(SDL_GL_LoadLibrary(NULL) != 0) {
		LogSdlError("[SDL] Failed to initialize OpenGL, attempting to continue with initialization.");
	}

	uint32_t baseFlags = _vsyncEnabled ? SDL_RENDERER_PRESENTVSYNC : 0;

	//TODO SDL_RENDERER_ACCELERATED ?
	_sdlRenderer = SDL_CreateRenderer(_sdlWindow, -1, baseFlags | SDL_RENDERER_SOFTWARE);
	if(!_sdlRenderer) {
		LogSdlError("[SDL] Failed to create accelerated renderer.");

		MessageManager::Log("[SDL] Attempting to create software renderer...");
		_sdlRenderer = SDL_CreateRenderer(_sdlWindow, -1, baseFlags | SDL_RENDERER_SOFTWARE);
		if(!_sdlRenderer) {
			LogSdlError("[SDL] Failed to create software renderer.");
			return false;
		}		
	}

	_sdlTexture = SDL_CreateTexture(_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, _frameWidth, _frameHeight);
	if(!_sdlTexture) {
		string msg = "[SDL] Failed to create texture: " + std::to_string(_frameWidth) + "x" + std::to_string(_frameHeight);
		LogSdlError(msg.c_str());
		return false;
	}

	SDL_SetWindowSize(_sdlWindow, _screenWidth, _screenHeight);

	return true;
}

void SdlRenderer::Cleanup()
{
	if(_sdlTexture) {
		SDL_DestroyTexture(_sdlTexture);
		_sdlTexture = nullptr;		
	}
	if(_sdlHudTexture) {
		SDL_DestroyTexture(_sdlHudTexture);
		_sdlHudTexture = nullptr;
	}
	if(_sdlRenderer) {
		SDL_DestroyRenderer(_sdlRenderer);
		_sdlRenderer = nullptr;
	}
}

void SdlRenderer::Reset()
{
	Cleanup();
	if(Init()) {
		_emu->GetVideoRenderer()->RegisterRenderingDevice(this);
	} else {
		Cleanup();
	}
}

void SdlRenderer::SetScreenSize(uint32_t width, uint32_t height)
{
	VideoConfig cfg = _emu->GetSettings()->GetVideoConfig();
	FrameInfo size = _emu->GetVideoRenderer()->GetRendererSize();
	if(_screenHeight != size.Height || _screenWidth != size.Width || _frameHeight != height || _frameWidth != width || _useBilinearInterpolation != cfg.UseBilinearInterpolation || _vsyncEnabled != cfg.VerticalSync) {
		_vsyncEnabled = cfg.VerticalSync;
		_useBilinearInterpolation = cfg.UseBilinearInterpolation;

		_frameHeight = height;
		_frameWidth = width;
		_newFrameBufferSize = width*height;

		_screenHeight = size.Height;
		_screenWidth = size.Width;

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, _useBilinearInterpolation ? "1" : "0");
		_screenBufferSize = _screenHeight*_screenWidth;

		Reset();
	}	
}

void SdlRenderer::ClearFrame()
{
	auto lock = _frameLock.AcquireSafe();
	if(_frameBuffer == nullptr) { 
		return;
	}

	memset(_frameBuffer, 0, _requiredWidth * _requiredHeight * _bytesPerPixel);
	_frameChanged = true;
}

void SdlRenderer::UpdateFrame(RenderedFrame& frame)
{
	auto lock = _frameLock.AcquireSafe();
	if(_frameBuffer == nullptr || _requiredWidth != frame.Width || _requiredHeight != frame.Height) {
		_requiredWidth = frame.Width;
		_requiredHeight = frame.Height;
		
		delete[] _frameBuffer;
		_frameBuffer = new uint32_t[frame.Width*frame.Height];
		memset(_frameBuffer, 0, frame.Width * frame.Height *4);
	}
	
	memcpy(_frameBuffer, frame.FrameBuffer, frame.Width * frame.Height *_bytesPerPixel);
	_frameChanged = true;	
}

void SdlRenderer::Render(uint32_t* hudBuffer, uint32_t width, uint32_t height)
{
	SetScreenSize(_requiredWidth, _requiredHeight);
	if(!_sdlRenderer || !_sdlTexture) {
		return;
	}

	if(!_sdlHudTexture || _hudWidth != width || _hudHeight != height) {
		if(_sdlHudTexture) {
			SDL_DestroyTexture(_sdlHudTexture);
		}
		_hudWidth = width;
		_hudHeight = height;
		_sdlHudTexture = SDL_CreateTexture(_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
		SDL_SetTextureBlendMode(_sdlHudTexture, SDL_BLENDMODE_BLEND);
	}

	if(SDL_RenderClear(_sdlRenderer) != 0) {
		LogSdlError("SDL_RenderClear failed");
	}

	uint8_t *textureBuffer;
	int rowPitch;
	if(SDL_LockTexture(_sdlTexture, nullptr, (void**)&textureBuffer, &rowPitch) == 0) {
		auto frameLock = _frameLock.AcquireSafe();
		if(_frameBuffer && _frameWidth == _requiredWidth && _frameHeight == _requiredHeight) {
			uint32_t* ppuFrameBuffer = _frameBuffer;
			for(uint32_t i = 0, iMax = _frameHeight; i < iMax; i++) {
				memcpy(textureBuffer, ppuFrameBuffer, _frameWidth*_bytesPerPixel);
				ppuFrameBuffer += _frameWidth;
				textureBuffer += rowPitch;
			}
		}
	} else {
		LogSdlError("SDL_LockTexture failed");
	}
	
	SDL_UnlockTexture(_sdlTexture);

	if(SDL_LockTexture(_sdlHudTexture, nullptr, (void**)&textureBuffer, &rowPitch) == 0) {
		for(uint32_t i = 0, iMax = height; i < iMax; i++) {
			memcpy(textureBuffer, hudBuffer, width * _bytesPerPixel);
			hudBuffer += width;
			textureBuffer += rowPitch;
		}
	} else {
		LogSdlError("SDL_LockTexture failed (HUD)");
	}

	SDL_UnlockTexture(_sdlHudTexture);

	SDL_Rect source = {0, 0, (int)_frameWidth, (int)_frameHeight };
	SDL_Rect dest = {0, 0, (int)_screenWidth, (int)_screenHeight };
	
	if(SDL_RenderCopy(_sdlRenderer, _sdlTexture, &source, &dest) != 0) {
			LogSdlError("SDL_RenderCopy failed");	
	}

	SDL_Rect hudSource = { 0, 0, (int)width, (int)height };
	if(SDL_RenderCopy(_sdlRenderer, _sdlHudTexture, &hudSource, &dest) != 0) {
		LogSdlError("SDL_RenderCopy failed (HUD)");
	}

	SDL_RenderPresent(_sdlRenderer);
}
