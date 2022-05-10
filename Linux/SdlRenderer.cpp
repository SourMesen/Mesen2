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
	Cleanup();
	delete[] _frameBuffer;	
}

void SdlRenderer::LogSdlError(const char* msg)
{
	MessageManager::Log(msg);
	MessageManager::Log(SDL_GetError());
}

void SdlRenderer::SetFullscreenMode(bool fullscreen, void* windowHandle, uint32_t monitorWidth, uint32_t monitorHeight)
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

	_sdlTexture = SDL_CreateTexture(_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, _nesFrameWidth, _nesFrameHeight);
	if(!_sdlTexture) {
		string msg = "[SDL] Failed to create texture: " + std::to_string(_nesFrameWidth) + "x" + std::to_string(_nesFrameHeight);
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
	if(_screenHeight != size.Height || _screenWidth != size.Width || _nesFrameHeight != height || _nesFrameWidth != width || _useBilinearInterpolation != cfg.UseBilinearInterpolation || _vsyncEnabled != cfg.VerticalSync) {
		_vsyncEnabled = cfg.VerticalSync;
		_useBilinearInterpolation = cfg.UseBilinearInterpolation;

		_nesFrameHeight = height;
		_nesFrameWidth = width;
		_newFrameBufferSize = width*height;

		_screenHeight = size.Height;
		_screenWidth = size.Width;

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, _useBilinearInterpolation ? "1" : "0");
		_screenBufferSize = _screenHeight*_screenWidth;

		Reset();
	}	
}

void SdlRenderer::UpdateFrame(RenderedFrame& frame)
{
	_frameLock.Acquire();
	if(_frameBuffer == nullptr || _requiredWidth != frame.Width || _requiredHeight != frame.Height) {
		_requiredWidth = frame.Width;
		_requiredHeight = frame.Height;
		
		delete[] _frameBuffer;
		_frameBuffer = new uint32_t[frame.Width*frame.Height];
		memset(_frameBuffer, 0, frame.Width * frame.Height *4);
	}
	
	memcpy(_frameBuffer, frame.FrameBuffer, frame.Width * frame.Height *_bytesPerPixel);
	_frameChanged = true;	
	_frameLock.Release();
}

void SdlRenderer::Render(uint32_t* hudBuffer, uint32_t width, uint32_t height)
{
	SetScreenSize(_requiredWidth, _requiredHeight);
	
	if(!_sdlRenderer || !_sdlTexture) {
		return;
	}

	if(SDL_RenderClear(_sdlRenderer) != 0) {
		LogSdlError("SDL_RenderClear failed");
	}

	uint8_t *textureBuffer;
	int rowPitch;
	if(SDL_LockTexture(_sdlTexture, nullptr, (void**)&textureBuffer, &rowPitch) == 0) {
		auto frameLock = _frameLock.AcquireSafe();
		if(_frameBuffer && _nesFrameWidth == _requiredWidth && _nesFrameHeight == _requiredHeight) {
			uint32_t* ppuFrameBuffer = _frameBuffer;
			for(uint32_t i = 0, iMax = _nesFrameHeight; i < iMax; i++) {
				memcpy(textureBuffer, ppuFrameBuffer, _nesFrameWidth*_bytesPerPixel);
				ppuFrameBuffer += _nesFrameWidth;
				textureBuffer += rowPitch;
			}
		}
	} else {
		LogSdlError("SDL_LockTexture failed");
	}
	
	SDL_UnlockTexture(_sdlTexture);

	SDL_Rect source = {0, 0, (int)_nesFrameWidth, (int)_nesFrameHeight };
	SDL_Rect dest = {0, 0, (int)_screenWidth, (int)_screenHeight };
	
	if(SDL_RenderCopy(_sdlRenderer, _sdlTexture, &source, &dest) != 0) {
			LogSdlError("SDL_RenderCopy failed");	
	}

	SDL_RenderPresent(_sdlRenderer);
}
