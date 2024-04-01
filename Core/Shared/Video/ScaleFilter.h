#pragma once

#include "pch.h"
#include "Shared/SettingTypes.h"

class Emulator;

class ScaleFilter
{
private:
	static bool _hqxInitDone;
	
	Emulator* _emu = nullptr;

	uint32_t _filterScale;
	ScaleFilterType _scaleFilterType;
	uint32_t *_outputBuffer = nullptr;
	uint32_t _width = 0;
	uint32_t _height = 0;

	uint32_t ApplyBrightness(uint32_t argb, uint8_t brightness);
	void ApplyLcdGridFilter(uint32_t* inputArgbBuffer);

	void ApplyPrescaleFilter(uint32_t *inputArgbBuffer);
	void UpdateOutputBuffer(uint32_t width, uint32_t height);

public:
	ScaleFilter(Emulator* emu, ScaleFilterType scaleFilterType, uint32_t scale);
	~ScaleFilter();

	uint32_t GetScale();
	uint32_t* ApplyFilter(uint32_t *inputArgbBuffer, uint32_t width, uint32_t height);
	FrameInfo GetFrameInfo(FrameInfo baseFrameInfo);

	static unique_ptr<ScaleFilter> GetScaleFilter(Emulator* emu, VideoFilterType filter);
};