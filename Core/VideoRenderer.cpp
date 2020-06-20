#include "stdafx.h"
#include "IRenderingDevice.h"
#include "VideoRenderer.h"
#include "VideoDecoder.h"
#include "Console.h"
#include "EmuSettings.h"
#include "MessageManager.h"
#include "../Utilities/IVideoRecorder.h"
#include "../Utilities/AviRecorder.h"
#include "../Utilities/GifRecorder.h"

VideoRenderer::VideoRenderer(shared_ptr<Console> console)
{
	_console = console;
	_stopFlag = false;	
	StartThread();
}

VideoRenderer::~VideoRenderer()
{
	_stopFlag = true;
	StopThread();
}

void VideoRenderer::StartThread()
{
#ifndef LIBRETRO
	if(!_renderThread) {
		_stopFlag = false;
		_waitForRender.Reset();

		_renderThread.reset(new std::thread(&VideoRenderer::RenderThread, this));
	}
#endif
}

void VideoRenderer::StopThread()
{
#ifndef LIBRETRO
	_stopFlag = true;
	if(_renderThread) {
		_renderThread->join();
		_renderThread.reset();
	}
#endif
}

void VideoRenderer::RenderThread()
{
	if(_renderer) {
		_renderer->Reset();
	}

	while(!_stopFlag.load()) {
		//Wait until a frame is ready, or until 16ms have passed (to allow UI to run at a minimum of 60fps)
		_waitForRender.Wait(16);
		if(_renderer) {
			_renderer->Render();
		}
	}
}

void VideoRenderer::UpdateFrame(void* frameBuffer, uint32_t width, uint32_t height)
{
	shared_ptr<IVideoRecorder> recorder = _recorder;
	if(recorder) {
		recorder->AddFrame(frameBuffer, width, height, _console->GetFps());
	}

	if(_renderer) {
		_renderer->UpdateFrame(frameBuffer, width, height);
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
	FrameInfo frameInfo = _console->GetVideoDecoder()->GetFrameInfo();

	shared_ptr<IVideoRecorder> recorder;
	if(codec == VideoCodec::GIF) {
		recorder.reset(new GifRecorder());
	} else {
		recorder.reset(new AviRecorder(codec, compressionLevel));
	}

	if(recorder->StartRecording(filename, frameInfo.Width, frameInfo.Height, 4, _console->GetSettings()->GetAudioConfig().SampleRate, _console->GetFps())) {
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