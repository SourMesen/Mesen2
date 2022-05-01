#pragma once
#include "stdafx.h"
#include "Utilities/SimpleLock.h"
#include "Shared/SettingTypes.h"

class Emulator;

class BaseVideoFilter
{
private:
	uint32_t* _outputBuffer = nullptr;
	double _yiqToRgbMatrix[6] = {};
	uint32_t _bufferSize = 0;
	SimpleLock _frameLock;
	OverscanDimensions _overscan = {};
	bool _isOddFrame = false;

	void UpdateBufferSize();

protected:
	Emulator* _emu = nullptr;
	FrameInfo _baseFrameInfo = {};
	FrameInfo _frameInfo = {};
	void* _frameData = nullptr;

	void InitConversionMatrix(double hueShift, double saturationShift);
	void RgbToYiq(double r, double g, double b, double& y, double& i, double& q);
	void YiqToRgb(double y, double i, double q, double& r, double& g, double& b);

	virtual void ApplyFilter(uint16_t *ppuOutputBuffer) = 0;
	virtual void OnBeforeApplyFilter();
	bool IsOddFrame();
	uint32_t GetBufferSize();
	virtual FrameInfo GetFrameInfo();
	
	template<typename T> bool NtscFilterOptionsChanged(T& ntscSetup);
	template<typename T> void InitNtscFilter(T& ntscSetup);

public:
	BaseVideoFilter(Emulator* emu);
	virtual ~BaseVideoFilter();

	uint32_t* GetOutputBuffer();
	FrameInfo SendFrame(uint16_t *ppuOutputBuffer, uint32_t frameNumber, void* frameData);
	void TakeScreenshot(string romName, VideoFilterType filterType);
	void TakeScreenshot(VideoFilterType filterType, string filename, std::stringstream *stream = nullptr);

	virtual OverscanDimensions GetOverscan();
	
	void SetBaseFrameInfo(FrameInfo frameInfo);
};
