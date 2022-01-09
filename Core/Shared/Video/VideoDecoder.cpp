#include "stdafx.h"
#include "Shared/Interfaces/IRenderingDevice.h"
#include "Shared/Video/VideoDecoder.h"
#include "Shared/Video/VideoRenderer.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Shared/NotificationManager.h"
#include "Shared/Emulator.h"
#include "Shared/RewindManager.h"
#include "Shared/EmuSettings.h"
#include "Shared/SettingTypes.h"
#include "Shared/Video/ScaleFilter.h"
#include "Shared/Video/DebugHud.h"
#include "Shared/InputHud.h"
#include "Shared/Video/SystemHud.h"
#include "SNES/CartTypes.h"

VideoDecoder::VideoDecoder(shared_ptr<Emulator> emu)
{
	_emu = emu;
	_frameChanged = false;
	_stopFlag = false;
	_baseFrameInfo = { 512, 478 };
	_lastFrameInfo = _baseFrameInfo;
	UpdateVideoFilter();
	_videoFilter->SetBaseFrameInfo(_baseFrameInfo);
	_inputHud.reset(new InputHud(emu.get()));
	_systemHud.reset(new SystemHud(emu.get()));
}

VideoDecoder::~VideoDecoder()
{
	StopThread();
}

FrameInfo VideoDecoder::GetBaseFrameInfo(bool removeOverscan)
{
	if(removeOverscan) {
		OverscanDimensions overscan = _emu->GetSettings()->GetOverscan();
		return { _baseFrameInfo.Width - overscan.Left - overscan.Right, _baseFrameInfo.Height - overscan.Top - overscan.Bottom };
	} else {
		return _baseFrameInfo;
	}
}

FrameInfo VideoDecoder::GetFrameInfo()
{
	return _lastFrameInfo;
}

void VideoDecoder::UpdateVideoFilter()
{
	VideoFilterType newFilter = _emu->GetSettings()->GetVideoConfig().VideoFilter;
	ConsoleType consoleType = _emu->GetConsoleType();

	if(_videoFilterType != newFilter || _videoFilter == nullptr || _consoleType != consoleType) {
		_videoFilterType = newFilter;
		_consoleType = consoleType;

		_videoFilter.reset(_emu->GetVideoFilter());

		if(_videoFilterType != VideoFilterType::None && _videoFilterType != VideoFilterType::NTSC) {
			_scaleFilter = ScaleFilter::GetScaleFilter(_videoFilterType);
		} else {
			_scaleFilter.reset();
		}
	}
}

void VideoDecoder::DecodeFrame(bool forRewind)
{
	UpdateVideoFilter();

	_videoFilter->SetBaseFrameInfo(_baseFrameInfo);
	_videoFilter->SendFrame(_ppuOutputBuffer, _frameNumber, _frameData);

	uint32_t* outputBuffer = _videoFilter->GetOutputBuffer();
	FrameInfo frameInfo = _videoFilter->GetFrameInfo();
	
	OverscanDimensions overscan = _videoFilter->GetOverscan();

	_inputHud->DrawControllers(overscan, _frameNumber);
	_systemHud->Draw(frameInfo, overscan);

	if(_scaleFilter) {
		outputBuffer = _scaleFilter->ApplyFilter(outputBuffer, frameInfo.Width, frameInfo.Height, _emu->GetSettings()->GetVideoConfig().ScanlineIntensity);
		frameInfo = _scaleFilter->GetFrameInfo(frameInfo);
		overscan.Left *= _scaleFilter->GetScale();
		overscan.Right *= _scaleFilter->GetScale();
		overscan.Top *= _scaleFilter->GetScale();
		overscan.Bottom *= _scaleFilter->GetScale();
	}

	_emu->GetDebugHud()->Draw(outputBuffer, frameInfo, overscan, _frameNumber);

	double aspectRatio = _emu->GetSettings()->GetAspectRatio(_emu->GetRegion(), _baseFrameInfo);
	if(frameInfo.Height != _lastFrameInfo.Height || frameInfo.Width != _lastFrameInfo.Width || aspectRatio != _lastAspectRatio) {
		_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::ResolutionChanged);
	}
	_lastAspectRatio = aspectRatio;
	_lastFrameInfo = frameInfo;

	//Rewind manager will take care of sending the correct frame to the video renderer
	_emu->GetRewindManager()->SendFrame(outputBuffer, frameInfo.Width, frameInfo.Height, forRewind);

	_frameChanged = false;
}

void VideoDecoder::DecodeThread()
{
	//This thread will decode the PPU's output (color ID to RGB, intensify r/g/b and produce a HD version of the frame if needed)
	while(!_stopFlag.load()) {
		//DecodeFrame returns the final ARGB frame we want to display in the emulator window
		while(!_frameChanged) {
			_waitForFrame.Wait();
			if(_stopFlag.load()) {
				return;
			}
		}

		DecodeFrame();
	}
}

uint32_t VideoDecoder::GetFrameCount()
{
	return _frameCount;
}

void VideoDecoder::UpdateFrame(uint16_t *ppuOutputBuffer, uint16_t width, uint16_t height, uint32_t frameNumber, bool sync, bool forRewind, void* frameData)
{
	if(_emu->IsRunAheadFrame()) {
		return;
	}

	_emu->OnBeforeSendFrame();

	if(_frameChanged) {
		//Last frame isn't done decoding yet - sometimes Signal() introduces a 25-30ms delay
		while(_frameChanged) {
			//Spin until decode is done
		}
		//At this point, we are sure that the decode thread is no longer busy
	}
	
	_baseFrameInfo.Width = width;
	_baseFrameInfo.Height = height;
	_frameNumber = frameNumber;
	_ppuOutputBuffer = ppuOutputBuffer;
	_frameData = frameData;
	if(sync) {
		DecodeFrame(forRewind);
	} else {
		_frameChanged = true;
		_waitForFrame.Signal();
	}
	_frameCount++;
}

void VideoDecoder::StartThread()
{
#ifndef LIBRETRO
	auto lock = _stopStartLock.AcquireSafe();
	if(!_decodeThread) {
		_videoFilter.reset();
		UpdateVideoFilter();
		_videoFilter->SetBaseFrameInfo(_baseFrameInfo);
		_stopFlag = false;
		_frameChanged = false;
		_frameCount = 0;
		_waitForFrame.Reset();
		
		_decodeThread.reset(new thread(&VideoDecoder::DecodeThread, this));
	}
#endif
}

void VideoDecoder::StopThread()
{
#ifndef LIBRETRO
	auto lock = _stopStartLock.AcquireSafe();
	_stopFlag = true;
	if(_decodeThread) {
		_waitForFrame.Signal();
		_decodeThread->join();

		_decodeThread.reset();

		//Clear whole screen
		if(_frameCount > 0) {
			vector<uint16_t> outputBuffer(512 * 478, 0);
			_ppuOutputBuffer = outputBuffer.data();
			memset(_ppuOutputBuffer, 0, 512 * 478 * 2);
			DecodeFrame();
			_ppuOutputBuffer = nullptr;
		}
	}
#endif
}

bool VideoDecoder::IsRunning()
{
	return _decodeThread != nullptr;
}

void VideoDecoder::TakeScreenshot()
{
	if(_videoFilter) {
		_videoFilter->TakeScreenshot(_emu->GetRomInfo().RomFile.GetFileName(), _videoFilterType);
	}
}

void VideoDecoder::TakeScreenshot(std::stringstream &stream)
{
	if(_videoFilter) {
		_videoFilter->TakeScreenshot(_videoFilterType, "", &stream);
	}
}
