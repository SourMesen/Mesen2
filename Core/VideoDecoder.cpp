#include "stdafx.h"
#include "IRenderingDevice.h"
#include "VideoDecoder.h"
#include "VideoRenderer.h"
#include "DefaultVideoFilter.h"
#include "Console.h"
#include "Ppu.h"
#include "DebugHud.h"

VideoDecoder::VideoDecoder(shared_ptr<Console> console)
{
	_console = console;
	_frameChanged = false;
	_stopFlag = false;
	UpdateVideoFilter();
}

VideoDecoder::~VideoDecoder()
{
	StopThread();
}

FrameInfo VideoDecoder::GetFrameInfo()
{
	return _lastFrameInfo;
}

void VideoDecoder::GetScreenSize(ScreenSize &size, bool ignoreScale)
{
	if(_videoFilter) {
		/*OverscanDimensions overscan = ignoreScale ? _videoFilter->GetOverscan() : _console->GetSettings()->GetOverscanDimensions();
		FrameInfo frameInfo{ overscan.GetScreenWidth(), overscan.GetScreenHeight(), PPU::ScreenWidth, PPU::ScreenHeight, 4 };
		double aspectRatio = _console->GetSettings()->GetAspectRatio(_console);
		double scale = (ignoreScale ? 1 : _console->GetSettings()->GetVideoScale());
		size.Width = (int32_t)(frameInfo.Width * scale);
		size.Height = (int32_t)(frameInfo.Height * scale);
		if(aspectRatio != 0.0) {
			size.Width = (uint32_t)(frameInfo.OriginalHeight * scale * aspectRatio * ((double)frameInfo.Width / frameInfo.OriginalWidth));
		}

		if(_console->GetSettings()->GetScreenRotation() % 180) {
			std::swap(size.Width, size.Height);
		}

		size.Scale = scale;*/
		
		size.Scale = 2;
		if(ignoreScale) {
			size.Width = _baseFrameInfo.Width;
			size.Height = _baseFrameInfo.Height;
		} else {
			if(_baseFrameInfo.Width == 256) {
				size.Width = (int32_t)(_baseFrameInfo.Width * size.Scale);
				size.Height = (int32_t)(_baseFrameInfo.Height * size.Scale);
			} else {
				size.Width = (int32_t)_baseFrameInfo.Width;
				size.Height = (int32_t)_baseFrameInfo.Height;
				if(_baseFrameInfo.Height <= 240) {
					size.Height *= 2;
				}
			}
		}
	}
}

void VideoDecoder::UpdateVideoFilter()
{
	VideoFilterType newFilter = VideoFilterType::None;

	if(_videoFilterType != newFilter || _videoFilter == nullptr) {
		_videoFilterType = newFilter;
		_videoFilter.reset(new DefaultVideoFilter(_console));
		//_scaleFilter.reset();

		switch(_videoFilterType) {
			case VideoFilterType::None: break;

			//TODO
			/*
			case VideoFilterType::NTSC: _videoFilter.reset(new NtscFilter(_console)); break;
			case VideoFilterType::BisqwitNtsc: _videoFilter.reset(new BisqwitNtscFilter(_console, 1)); break;
			case VideoFilterType::BisqwitNtscHalfRes: _videoFilter.reset(new BisqwitNtscFilter(_console, 2)); break;
			case VideoFilterType::BisqwitNtscQuarterRes: _videoFilter.reset(new BisqwitNtscFilter(_console, 4)); break;
			case VideoFilterType::Raw: _videoFilter.reset(new RawVideoFilter(_console)); break;
			default: _scaleFilter = ScaleFilter::GetScaleFilter(_videoFilterType); break;*/
		}
	}

	//TODO
	/*	
	if(_console->GetSettings()->GetScreenRotation() == 0 && _rotateFilter) {
		_rotateFilter.reset();
	} else if(_console->GetSettings()->GetScreenRotation() > 0) {
		if(!_rotateFilter || _rotateFilter->GetAngle() != _console->GetSettings()->GetScreenRotation()) {
			_rotateFilter.reset(new RotateFilter(_console->GetSettings()->GetScreenRotation()));
		}
	}*/
}

void VideoDecoder::DecodeFrame(bool synchronous)
{
	UpdateVideoFilter();

	_videoFilter->SetBaseFrameInfo(_baseFrameInfo);
	_videoFilter->SendFrame(_ppuOutputBuffer, _frameNumber);

	uint32_t* outputBuffer = _videoFilter->GetOutputBuffer();
	FrameInfo frameInfo = _videoFilter->GetFrameInfo();
	_console->GetDebugHud()->Draw(outputBuffer, _videoFilter->GetOverscan(), frameInfo.Width, _frameNumber);

	/*
	if(_rotateFilter) {
		outputBuffer = _rotateFilter->ApplyFilter(outputBuffer, frameInfo.Width, frameInfo.Height);
		frameInfo = _rotateFilter->GetFrameInfo(frameInfo);
	}

	if(_scaleFilter) {
		outputBuffer = _scaleFilter->ApplyFilter(outputBuffer, frameInfo.Width, frameInfo.Height, _console->GetSettings()->GetPictureSettings().ScanlineIntensity);
		frameInfo = _scaleFilter->GetFrameInfo(frameInfo);
	}

	if(_hud) {
		_hud->DrawHud(_console, outputBuffer, frameInfo, _videoFilter->GetOverscan());
	}*/

	ScreenSize screenSize;
	GetScreenSize(screenSize, true);
	/*if(_previousScale != _console->GetSettings()->GetVideoScale() || screenSize.Height != _previousScreenSize.Height || screenSize.Width != _previousScreenSize.Width) {
		_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::ResolutionChanged);
	}*/
	_previousScale = 1; // _console->GetSettings()->GetVideoScale();
	_previousScreenSize = screenSize;
	_lastFrameInfo = frameInfo;

	_frameChanged = false;
	
	//Rewind manager will take care of sending the correct frame to the video renderer
	_console->GetVideoRenderer()->UpdateFrame(outputBuffer, frameInfo.Width, frameInfo.Height);
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

void VideoDecoder::UpdateFrameSync(uint16_t *ppuOutputBuffer, uint16_t width, uint16_t height, uint32_t frameNumber)
{
	_baseFrameInfo.Width = width;
	_baseFrameInfo.Height = height;
	_frameNumber = frameNumber;
	_ppuOutputBuffer = ppuOutputBuffer;
	DecodeFrame(true);
	_frameCount++;
}

void VideoDecoder::UpdateFrame(uint16_t *ppuOutputBuffer, uint16_t width, uint16_t height, uint32_t frameNumber)
{
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
	_frameChanged = true;
	_waitForFrame.Signal();

	_frameCount++;
}

void VideoDecoder::StartThread()
{
#ifndef LIBRETRO
	if(!_decodeThread) {	
		_stopFlag = false;
		_frameChanged = false;
		_frameCount = 0;
		_waitForFrame.Reset();
		
		//TODO
		//_hud.reset(new VideoHud());

		_decodeThread.reset(new thread(&VideoDecoder::DecodeThread, this));
	}
#endif
}

void VideoDecoder::StopThread()
{
#ifndef LIBRETRO
	_stopFlag = true;
	if(_decodeThread) {
		_waitForFrame.Signal();
		_decodeThread->join();

		_decodeThread.reset();

		
		//TODO
		//_hud.reset();
		/*UpdateVideoFilter();
		if(_ppuOutputBuffer != nullptr) {
			//Clear whole screen
			for(uint32_t i = 0; i < PPU::PixelCount; i++) {
				_ppuOutputBuffer[i] = 14; //Black
			}
			DecodeFrame();
		}*/
		_ppuOutputBuffer = nullptr;
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
		//TODO
		//_videoFilter->TakeScreenshot(_console->GetRomPath().GetFileName(), _videoFilterType);
	}
}

void VideoDecoder::TakeScreenshot(std::stringstream &stream)
{
	if(_videoFilter) {
		_videoFilter->TakeScreenshot(_videoFilterType, "", &stream);
	}
}
