#pragma once
#include "stdafx.h"
#include "Utilities/SimpleLock.h"
#include "SettingTypes.h"

class Emulator;

class BaseVideoFilter
{
private:
	uint32_t* _outputBuffer = nullptr;
	double _yiqToRgbMatrix[6] = {};
	uint32_t _bufferSize = 0;
	SimpleLock _frameLock;
	OverscanDimensions _overscan;
	bool _isOddFrame;

	void UpdateBufferSize();

protected:
	shared_ptr<Emulator> _emu;
	FrameInfo _baseFrameInfo;

	void InitConversionMatrix(double hueShift, double saturationShift);
	void RgbToYiq(double r, double g, double b, double& y, double& i, double& q);
	void YiqToRgb(double y, double i, double q, double& r, double& g, double& b);

	virtual void ApplyFilter(uint16_t *ppuOutputBuffer) = 0;
	virtual void OnBeforeApplyFilter();
	bool IsOddFrame();
	uint32_t GetBufferSize();
	uint32_t ApplyScanlineEffect(uint32_t argb, uint8_t scanlineIntensity);

public:
	BaseVideoFilter(shared_ptr<Emulator> emu);
	virtual ~BaseVideoFilter();

	uint32_t* GetOutputBuffer();
	void SendFrame(uint16_t *ppuOutputBuffer, uint32_t frameNumber);
	void TakeScreenshot(string romName, VideoFilterType filterType);
	void TakeScreenshot(VideoFilterType filterType, string filename, std::stringstream *stream = nullptr);

	virtual OverscanDimensions GetOverscan();
	
	void SetBaseFrameInfo(FrameInfo frameInfo);
	virtual FrameInfo GetFrameInfo();
};