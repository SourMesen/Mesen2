#include "pch.h"
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
	_systemHud.reset(new SystemHud(_emu));
	_inputHud.reset(new InputHud(emu, _rendererHud.get()));
}

VideoRenderer::~VideoRenderer()
{
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
	if(!_renderThread) {
		auto lock = _stopStartLock.AcquireSafe();
		if(!_renderThread) {
			_stopFlag = false;
			_waitForRender.Reset();

			_renderThread.reset(new std::thread(&VideoRenderer::RenderThread, this));
		}
	}
}

void VideoRenderer::StopThread()
{
	_stopFlag = true;
	if(_renderThread) {
		auto lock = _stopStartLock.AcquireSafe();
		if(_renderThread) {
			_renderThread->join();
			_renderThread.reset();
		}
	}
}

void VideoRenderer::RenderThread()
{
	if(_renderer) {
		_renderer->OnRendererThreadStarted();
	}

	while(!_stopFlag.load()) {
		//Wait until a frame is ready, or until 32ms have passed (to allow HUD to update at ~30fps when paused)
		bool forceRender = !_waitForRender.Wait(32);
		if(_renderer) {
			FrameInfo size = _emu->GetVideoDecoder()->GetBaseFrameInfo(true);
			_scriptHudSurface.UpdateSize(size.Width * _scriptHudScale, size.Height * _scriptHudScale);

			size = GetEmuHudSize(size);
			if(_emuHudSurface.UpdateSize(size.Width, size.Height)) {
				_rendererHud->ClearScreen();
			}

			RenderedFrame frame;
			{
				auto lock = _frameLock.AcquireSafe();
				frame = _lastFrame;
			}

			_inputHud->DrawControllers(size, frame.InputData);
			{
				auto lock = _hudLock.AcquireSafe();
				_systemHud->Draw(_rendererHud.get(), size.Width, size.Height);
			}
			
			_emuHudSurface.IsDirty = _rendererHud->Draw(_emuHudSurface.Buffer, size, {}, 0, {}, true);
			_scriptHudSurface.IsDirty = DrawScriptHud(frame);

			if(forceRender || _needRedraw || _emuHudSurface.IsDirty || _scriptHudSurface.IsDirty) {
				_needRedraw = false;
				_renderer->Render(_emuHudSurface, _scriptHudSurface);
			}
		}
	}
}

FrameInfo VideoRenderer::GetEmuHudSize(FrameInfo baseFrameSize)
{
	FrameInfo size = {};
	if(_emu->GetSettings()->GetPreferences().HudSize == HudDisplaySize::Scaled) {
		//Adjust the system HUD's width to match the aspect ratio to allow text to be unstretched
		//(The Lua HUD is not adjusted to allow scripts that need to match positions on the game screen to work correctly.)
		double aspectRatio = _emu->GetSettings()->GetAspectRatio(_emu->GetRegion(), baseFrameSize);
		size.Width = (uint32_t)std::round(baseFrameSize.Height * aspectRatio);
		size.Height = baseFrameSize.Height;
	} else {
		size.Width = _rendererWidth / 2;
		size.Height = _rendererHeight / 2;
	}
	return size;
}

bool VideoRenderer::DrawScriptHud(RenderedFrame& frame)
{
	bool needRedraw = false;
	if(_lastScriptHudFrameNumber != frame.FrameNumber) {
		//Clear+draw HUD for scripts
		//-Only when frame number changes (to prevent the HUD from disappearing when paused, etc.)
		//-Only when commands are queued, otherwise skip drawing/clearing to avoid wasting CPU time
		if(_needScriptHudClear) {
			_scriptHudSurface.Clear();
			_needScriptHudClear = false;
			needRedraw = true;
		}

		if(_emu->GetScriptHud()->HasCommands()) {
			auto [size, overscan] = GetScriptHudSize();
			_emu->GetScriptHud()->Draw(_scriptHudSurface.Buffer, size, overscan, frame.FrameNumber, {});
			_needScriptHudClear = true;
			_lastScriptHudFrameNumber = frame.FrameNumber;
			needRedraw = true;
		}
	}
	return needRedraw;
}

std::pair<FrameInfo, OverscanDimensions> VideoRenderer::GetScriptHudSize()
{
	FrameInfo scriptHudSize = { _scriptHudSurface.Width, _scriptHudSurface.Height };
	OverscanDimensions overscan = _emu->GetSettings()->GetOverscan();
	overscan.Top *= _scriptHudScale;
	overscan.Bottom *= _scriptHudScale;
	overscan.Left *= _scriptHudScale;
	overscan.Right *= _scriptHudScale;
	return { scriptHudSize, overscan };
}

void VideoRenderer::UpdateFrame(RenderedFrame& frame)
{
	{
		auto lock = _hudLock.AcquireSafe();
		_systemHud->UpdateHud();
	}

	ProcessAviRecording(frame);

	{
		auto lock = _frameLock.AcquireSafe();
		_lastFrame = frame;
	}

	if(_renderer) {
		_renderer->UpdateFrame(frame);
		_needRedraw = true;
		_waitForRender.Signal();
	}
}

void VideoRenderer::ClearFrame()
{
	if(_renderer) {
		_renderer->ClearFrame();
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

void VideoRenderer::ProcessAviRecording(RenderedFrame& frame)
{
	shared_ptr<IVideoRecorder> recorder = _recorder.lock();
	if(recorder) {
		if(!recorder->IsRecording()) {
			recorder->StartRecording(frame.Width, frame.Height, 4, _emu->GetSettings()->GetAudioConfig().SampleRate, _emu->GetFps());
		}

		if(_recorderOptions.RecordInputHud || _recorderOptions.RecordSystemHud) {
			//Calculate the scale needed for the HUD elements
			FrameInfo originalSize = _emu->GetVideoDecoder()->GetBaseFrameInfo(true);
			double scale = (double)frame.Height / originalSize.Height;
			FrameInfo scaledFrameSize = { (uint32_t)(frame.Width / scale), (uint32_t)(frame.Height / scale) };

			//Update the surface to match the frame's size
			_aviRecorderSurface.UpdateSize(frame.Width, frame.Height);
			
			//Copy the game screen
			memcpy(_aviRecorderSurface.Buffer, frame.FrameBuffer, frame.Width * frame.Height * sizeof(uint32_t));

			//Draw the system/input HUDs
			DebugHud hud;
			InputHud inputHud(_emu, &hud);
			if(_recorderOptions.RecordSystemHud) {
				_systemHud->Draw(&hud, scaledFrameSize.Width, scaledFrameSize.Height);
			}
			if(_recorderOptions.RecordInputHud) {
				inputHud.DrawControllers(scaledFrameSize, frame.InputData);
			}

			FrameInfo frameSize = { frame.Width, frame.Height };
			hud.Draw((uint32_t*)_aviRecorderSurface.Buffer, frameSize, {}, frame.FrameNumber, { scale, scale });

			//Record the final result
			if(!recorder->AddFrame(_aviRecorderSurface.Buffer, frame.Width, frame.Height, _emu->GetFps())) {
				StopRecording();
			}
		} else {
			//Only record the game screen
			if(!recorder->AddFrame(frame.FrameBuffer, frame.Width, frame.Height, _emu->GetFps())) {
				StopRecording();
			}
		}
	}
}

void VideoRenderer::StartRecording(string filename, RecordAviOptions options)
{
	_recorderOptions = options;

	shared_ptr<IVideoRecorder> recorder;
	if(options.Codec == VideoCodec::GIF) {
		recorder.reset(new GifRecorder());
	} else {
		recorder.reset(new AviRecorder(options.Codec, options.CompressionLevel));
	}

	if(recorder->Init(filename)) {
		_recorder.reset(recorder);
		
		if(!options.RecordSystemHud) {
			//Only display message if not recording the system HUD (otherwise the message is always visible on the recording, which isn't ideal)
			MessageManager::DisplayMessage("VideoRecorder", "VideoRecorderStarted", filename);
		}
	} else {
		MessageManager::DisplayMessage("VideoRecorder", "CouldNotWriteToFile", filename);
	}
}

void VideoRenderer::AddRecordingSound(int16_t* soundBuffer, uint32_t sampleCount, uint32_t sampleRate)
{
	shared_ptr<IVideoRecorder> recorder = _recorder.lock();
	if(recorder) {
		if(!recorder->AddSound(soundBuffer, sampleCount, sampleRate)) {
			StopRecording();
		}
	}
}

void VideoRenderer::StopRecording()
{
	shared_ptr<IVideoRecorder> recorder = _recorder.lock();
	if(recorder) {
		MessageManager::DisplayMessage("VideoRecorder", "VideoRecorderStopped", recorder->GetOutputFile());
	}
	_aviRecorderSurface.UpdateSize(0, 0);
	_recorder.reset();
}

bool VideoRenderer::IsRecording()
{
	return _recorder != nullptr;
}