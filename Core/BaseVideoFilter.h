#pragma once
#include "stdafx.h"
#include "../Utilities/SimpleLock.h"
#include "SettingTypes.h"

class Console;

class BaseVideoFilter
{
private:
	uint32_t* _outputBuffer = nullptr;
	uint32_t _bufferSize = 0;
	SimpleLock _frameLock;
	OverscanDimensions _overscan;
	bool _isOddFrame;

	void UpdateBufferSize();

protected:
	shared_ptr<Console> _console;
	FrameInfo _baseFrameInfo;

	virtual void ApplyFilter(uint16_t *ppuOutputBuffer) = 0;
	virtual void OnBeforeApplyFilter();
	bool IsOddFrame();
	uint32_t ApplyScanlineEffect(uint32_t argb, uint8_t scanlineIntensity);

public:
	BaseVideoFilter(shared_ptr<Console> console);
	virtual ~BaseVideoFilter();

	uint32_t* GetOutputBuffer();
	void SendFrame(uint16_t *ppuOutputBuffer, uint32_t frameNumber);
	void TakeScreenshot(string romName, VideoFilterType filterType);
	void TakeScreenshot(VideoFilterType filterType, string filename, std::stringstream *stream = nullptr);

	virtual OverscanDimensions GetOverscan();
	
	void SetBaseFrameInfo(FrameInfo frameInfo);
	virtual FrameInfo GetFrameInfo();
};