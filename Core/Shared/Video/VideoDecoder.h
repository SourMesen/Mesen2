#pragma once
#include "stdafx.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/AutoResetEvent.h"
#include "Shared/SettingTypes.h"

class BaseVideoFilter;
class ScaleFilter;
//class RotateFilter;
class IRenderingDevice;
class InputHud;
class SystemHud;
class Emulator;

class VideoDecoder
{
private:
	shared_ptr<Emulator> _emu;

	uint16_t *_ppuOutputBuffer = nullptr;
	void* _frameData = nullptr;
	uint32_t _frameNumber = 0;
	ConsoleType _consoleType = ConsoleType::Snes;

	unique_ptr<thread> _decodeThread;
	unique_ptr<InputHud> _inputHud;
	unique_ptr<SystemHud> _systemHud;

	SimpleLock _stopStartLock;
	AutoResetEvent _waitForFrame;
	
	atomic<bool> _frameChanged;
	atomic<bool> _stopFlag;
	uint32_t _frameCount = 0;

	double _lastAspectRatio = 0.0;
	FrameInfo _baseFrameInfo = {};
	FrameInfo _lastFrameInfo = {};

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
	FrameInfo GetBaseFrameInfo(bool removeOverscan);
	FrameInfo GetFrameInfo();

	void UpdateFrame(uint16_t *ppuOutputBuffer, uint16_t width, uint16_t height, uint32_t frameNumber, bool sync, bool forRewind, void* frameData = nullptr);

	bool IsRunning();
	void StartThread();
	void StopThread();
};