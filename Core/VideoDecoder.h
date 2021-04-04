#pragma once
#include "stdafx.h"
#include "../Utilities/SimpleLock.h"
#include "../Utilities/AutoResetEvent.h"
#include "SettingTypes.h"

class BaseVideoFilter;
class ScaleFilter;
//class RotateFilter;
class IRenderingDevice;
class InputHud;
class Emulator;

class VideoDecoder
{
private:
	shared_ptr<Emulator> _emu;

	uint16_t *_ppuOutputBuffer = nullptr;
	uint32_t _frameNumber = 0;

	unique_ptr<thread> _decodeThread;
	unique_ptr<InputHud> _inputHud;

	AutoResetEvent _waitForFrame;
	
	atomic<bool> _frameChanged;
	atomic<bool> _stopFlag;
	uint32_t _frameCount = 0;

	ScreenSize _previousScreenSize = {};
	double _previousScale = 0;
	FrameInfo _baseFrameInfo;
	FrameInfo _lastFrameInfo;

	VideoFilterType _videoFilterType = VideoFilterType::None;
	unique_ptr<BaseVideoFilter> _videoFilter;
	shared_ptr<ScaleFilter> _scaleFilter;
	//shared_ptr<RotateFilter> _rotateFilter;

	void UpdateVideoFilter();

	void DecodeThread();

public:
	VideoDecoder(shared_ptr<Emulator> console);
	~VideoDecoder();

	void DecodeFrame(bool synchronous = false);
	void TakeScreenshot();
	void TakeScreenshot(std::stringstream &stream);

	uint32_t GetFrameCount();

	FrameInfo GetFrameInfo();
	ScreenSize GetScreenSize(bool ignoreScale);

	void UpdateFrameSync(uint16_t *ppuOutputBuffer, uint16_t width, uint16_t height, uint32_t frameNumber, bool forRewind);
	void UpdateFrame(uint16_t *ppuOutputBuffer, uint16_t width, uint16_t height, uint32_t frameNumber);

	bool IsRunning();
	void StartThread();
	void StopThread();
};