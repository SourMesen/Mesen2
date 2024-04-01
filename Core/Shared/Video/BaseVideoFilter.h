#pragma once
#include "pch.h"
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
	uint32_t _videoPhase = 0;

	void UpdateBufferSize();

protected:
	Emulator* _emu = nullptr;
	FrameInfo _baseFrameInfo = {};
	FrameInfo _frameInfo = {};
	void* _frameData = nullptr;
	uint16_t* _ppuOutputBuffer = nullptr;

	void InitConversionMatrix(double hueShift, double saturationShift);
	void ApplyColorOptions(uint8_t& r, uint8_t& g, uint8_t& b, double brightness, double contrast);
	void RgbToYiq(double r, double g, double b, double& y, double& i, double& q);
	void YiqToRgb(double y, double i, double q, double& r, double& g, double& b);

	virtual void ApplyFilter(uint16_t *ppuOutputBuffer) = 0;
	virtual void OnBeforeApplyFilter();
	bool IsOddFrame();
	uint32_t GetVideoPhase();
	uint32_t GetBufferSize();

protected:
	virtual FrameInfo GetFrameInfo();

public:
	BaseVideoFilter(Emulator* emu);
	virtual ~BaseVideoFilter();

	template<typename T> bool NtscFilterOptionsChanged(T& ntscSetup, VideoConfig& cfg);
	template<typename T> static void InitNtscFilter(T& ntscSetup, VideoConfig& cfg);

	uint32_t* GetOutputBuffer();
	FrameInfo SendFrame(uint16_t *ppuOutputBuffer, uint32_t frameNumber, uint32_t videoPhase, void* frameData, bool enableOverscan = true);
	void TakeScreenshot(string romName, VideoFilterType filterType);
	void TakeScreenshot(VideoFilterType filterType, string filename, std::stringstream *stream = nullptr);

	virtual HudScaleFactors GetScaleFactor() { return { 1.0, 1.0 }; }
	virtual OverscanDimensions GetOverscan();
	void SetOverscan(OverscanDimensions dimensions);
	FrameInfo GetFrameInfo(uint16_t* ppuOutputBuffer, bool enableOverscan);

	void SetBaseFrameInfo(FrameInfo frameInfo);
};
