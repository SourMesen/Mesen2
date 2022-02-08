#include "stdafx.h"
#include "Shared/Video/VideoRenderer.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/Interfaces/IRenderingDevice.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Video/DebugHud.h"
#include "Shared/Video/SystemHud.h"
#include "Shared/InputHud.h"
#include "Shared/MessageManager.h"
#include "Utilities/Video/IVideoRecorder.h"
#include "Utilities/Video/AviRecorder.h"
#include "Utilities/Video/GifRecorder.h"

VideoRenderer::VideoRenderer(Emulator* emu)
{
	_emu = emu;
	_stopFlag = false;

	_rendererHud.reset(new DebugHud());
	_systemHud.reset(new SystemHud(_emu, _rendererHud.get()));

	_hudSurface = new uint32_t[256*240];
	_hudSize.Width = 256;
	_hudSize.Height = 240;
}

VideoRenderer::~VideoRenderer()
{
	delete[] _hudSurface;

	_stopFlag = true;
	StopThread();
}

FrameInfo VideoRenderer::GetRendererSize()
{
	FrameInfo frame = {};
	frame.Width = _rendererWidth;
	frame.Height = _rendererHeight;
	return frame;
}

void VideoRenderer::SetRendererSize(uint32_t width, uint32_t height)
{
	_rendererWidth = width;
	_rendererHeight = height;
}

void VideoRenderer::StartThread()
{
#ifndef LIBRETRO
	if(!_renderThread) {
		auto lock = _stopStartLock.AcquireSafe();
		if(!_renderThread) {
			_stopFlag = false;
			_waitForRender.Reset();

			_renderThread.reset(new std::thread(&VideoRenderer::RenderThread, this));
		}
	}
#endif
}

void VideoRenderer::StopThread()
{
#ifndef LIBRETRO
	_stopFlag = true;
	if(_renderThread) {
		auto lock = _stopStartLock.AcquireSafe();
		if(_renderThread) {
			_renderThread->join();
			_renderThread.reset();
		}
	}
#endif
}

void VideoRenderer::RenderThread()
{
	while(!_stopFlag.load()) {
		//Wait until a frame is ready, or until 32ms have passed (to allow HUD to update at ~30fps when paused)
		_waitForRender.Wait(32);
		if(_renderer) {
			FrameInfo size = _emu->GetVideoDecoder()->GetBaseFrameInfo(true);
			if(_hudSize.Width != size.Width || _hudSize.Height != size.Height) {
				delete[] _hudSurface;
				_hudSurface = new uint32_t[size.Height * size.Width];
				_hudSize = size;
			}

			memset(_hudSurface, 0, _hudSize.Width * _hudSize.Height * sizeof(uint32_t));
			_systemHud->Draw(_hudSize.Width, _hudSize.Height);
			_rendererHud->Draw(_hudSurface, _hudSize, {}, 0, false);
			_renderer->Render(_hudSurface, _hudSize.Width, _hudSize.Height);
		}
	}
}

void VideoRenderer::UpdateFrame(RenderedFrame frame)
{
	shared_ptr<IVideoRecorder> recorder = _recorder;
	if(recorder) {
		recorder->AddFrame(frame.FrameBuffer, frame.Width, frame.Height, _emu->GetFps());
	}

	if(_renderer) {
		_renderer->UpdateFrame(frame);
		_waitForRender.Signal();
	}
}

void VideoRenderer::RegisterRenderingDevice(IRenderingDevice *renderer)
{
	_renderer = renderer;
	StartThread();
}

void VideoRenderer::UnregisterRenderingDevice(IRenderingDevice *renderer)
{
	if(_renderer == renderer) {
		StopThread();
		_renderer = nullptr;
	}
}

void VideoRenderer::StartRecording(string filename, VideoCodec codec, uint32_t compressionLevel)
{
	FrameInfo frameInfo = _emu->GetVideoDecoder()->GetFrameInfo();

	shared_ptr<IVideoRecorder> recorder;
	if(codec == VideoCodec::GIF) {
		recorder.reset(new GifRecorder());
	} else {
		recorder.reset(new AviRecorder(codec, compressionLevel));
	}

	if(recorder->StartRecording(filename, frameInfo.Width, frameInfo.Height, 4, _emu->GetSettings()->GetAudioConfig().SampleRate, _emu->GetFps())) {
		_recorder = recorder;
		MessageManager::DisplayMessage("VideoRecorder", "VideoRecorderStarted", filename);
	}
}

void VideoRenderer::AddRecordingSound(int16_t* soundBuffer, uint32_t sampleCount, uint32_t sampleRate)
{
	shared_ptr<IVideoRecorder> recorder = _recorder;
	if(recorder) {
		recorder->AddSound(soundBuffer, sampleCount, sampleRate);
	}
}

void VideoRenderer::StopRecording()
{
	shared_ptr<IVideoRecorder> recorder = _recorder;
	if(recorder) {
		MessageManager::DisplayMessage("VideoRecorder", "VideoRecorderStopped", recorder->GetOutputFile());
	}
	_recorder.reset();
}

bool VideoRenderer::IsRecording()
{
	return _recorder != nullptr && _recorder->IsRecording();
}