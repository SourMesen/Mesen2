#include "SdlRenderer.h"
#include "Core/Debugger/Debugger.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/Video/VideoRenderer.h"
#include "Core/Shared/Video/VideoDecoder.h"
#include "Core/Shared/EmuSettings.h"
#include "Core/Shared/MessageManager.h"
#include "Core/Shared/RenderedFrame.h"

SimpleLock SdlRenderer::_frameLock;

SdlRenderer::SdlRenderer(Emulator* emu, void* windowHandle) : _windowHandle(windowHandle)
{
	_emu = emu;
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

	_sdlRenderer = SDL_CreateRenderer(_sdlWindow, -1, baseFlags | SDL_RENDERER_ACCELERATED);
	if(!_sdlRenderer) {
		LogSdlError("[SDL] Failed to create accelerated renderer.");

		MessageManager::Log("[SDL] Attempting to create software renderer...");
		_sdlRenderer = SDL_CreateRenderer(_sdlWindow, -1, baseFlags | SDL_RENDERER_SOFTWARE);
		if(!_sdlRenderer) {
			LogSdlError("[SDL] Failed to create software renderer.");
			return false;
		}
	}

	return true;
}

bool SdlRenderer::InitTexture()
{
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
	if(_emuHud.Texture) {
		SDL_DestroyTexture(_emuHud.Texture);
		_emuHud.Texture = nullptr;
	}
	if(_scriptHud.Texture) {
		SDL_DestroyTexture(_scriptHud.Texture);
		_scriptHud.Texture = nullptr;
	}
	if(_sdlRenderer) {
		SDL_DestroyRenderer(_sdlRenderer);
		_sdlRenderer = nullptr;
	}
}

void SdlRenderer::OnRendererThreadStarted()
{
	//SDL stops working if the rendering moves to a new thread
	//Reset everything to make it work again
	Reset();
}

void SdlRenderer::Reset()
{
	Cleanup();
	if(Init()) {
		InitTexture();
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

bool SdlRenderer::UpdateHudSize(HudRenderInfo& hud, uint32_t width, uint32_t height)
{
	if(!hud.Texture || hud.Width != width || hud.Height != height) {
		if(hud.Texture) {
			SDL_DestroyTexture(hud.Texture);
		}
		hud.Width = width;
		hud.Height = height;
		hud.Texture = SDL_CreateTexture(_sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
		SDL_SetTextureBlendMode(hud.Texture, SDL_BLENDMODE_BLEND);
		return true;
	}
	return false;
}

void SdlRenderer::UpdateHudTexture(HudRenderInfo& hud, uint32_t* src)
{
	uint8_t* textureBuffer;
	int rowPitch;
	if(SDL_LockTexture(hud.Texture, nullptr, (void**)&textureBuffer, &rowPitch) == 0) {
		for(uint32_t i = 0, iMax = hud.Height; i < iMax; i++) {
			memcpy(textureBuffer, src, hud.Width * _bytesPerPixel);
			src += hud.Width;
			textureBuffer += rowPitch;
		}
	} else {
		LogSdlError("SDL_LockTexture failed (HUD)");
	}
	SDL_UnlockTexture(hud.Texture);
}

void SdlRenderer::Render(RenderSurfaceInfo& emuHud, RenderSurfaceInfo& scriptHud)
{
	SetScreenSize(_requiredWidth, _requiredHeight);
	if(!_sdlRenderer || !_sdlTexture) {
		return;
	}

	bool needUpdate = false;
	needUpdate |= UpdateHudSize(_emuHud, emuHud.Width, emuHud.Height);
	needUpdate |= UpdateHudSize(_scriptHud, scriptHud.Width, scriptHud.Height);

	if(SDL_RenderClear(_sdlRenderer) != 0) {
		LogSdlError("SDL_RenderClear failed");
	}

	uint8_t *textureBuffer;
	int rowPitch;
	if(SDL_LockTexture(_sdlTexture, nullptr, (void**)&textureBuffer, &rowPitch) == 0) {
		auto frameLock = _frameLock.AcquireSafe();
		if(_frameBuffer && _frameWidth == _requiredWidth && _frameHeight == _requiredHeight) {
			uint32_t* ppuFrameBuffer = _frameBuffer;
			if(rowPitch != _frameWidth) {
				for(uint32_t i = 0, iMax = _frameHeight; i < iMax; i++) {
					memcpy(textureBuffer, ppuFrameBuffer, _frameWidth*_bytesPerPixel);
					ppuFrameBuffer += _frameWidth;
					textureBuffer += rowPitch;
				}
			} else {
				memcpy(textureBuffer, ppuFrameBuffer, _frameHeight * _frameWidth * _bytesPerPixel);
			}
		}
	} else {
		LogSdlError("SDL_LockTexture failed");
	}
	
	SDL_UnlockTexture(_sdlTexture);

	if(needUpdate || emuHud.IsDirty) {
		UpdateHudTexture(_emuHud, emuHud.Buffer);
	}
	if(needUpdate || scriptHud.IsDirty) {
		UpdateHudTexture(_scriptHud, scriptHud.Buffer);
	}

	SDL_Rect source = {0, 0, (int)_frameWidth, (int)_frameHeight };
	SDL_Rect dest = {0, 0, (int)_screenWidth, (int)_screenHeight };
	
	if(SDL_RenderCopy(_sdlRenderer, _sdlTexture, &source, &dest) != 0) {
		LogSdlError("SDL_RenderCopy failed");	
	}

	SDL_Rect scriptHudSource = { 0, 0, (int)_scriptHud.Width, (int)_scriptHud.Height };
	if(SDL_RenderCopy(_sdlRenderer, _scriptHud.Texture, &scriptHudSource, &dest) != 0) {
		LogSdlError("SDL_RenderCopy failed (_scriptHud)");
	}

	SDL_Rect emuHudSource = { 0, 0, (int)_emuHud.Width, (int)_emuHud.Height };
	if(SDL_RenderCopy(_sdlRenderer, _emuHud.Texture, &emuHudSource, &dest) != 0) {
		LogSdlError("SDL_RenderCopy failed (_emuHud)");
	}

	SDL_RenderPresent(_sdlRenderer);
}
