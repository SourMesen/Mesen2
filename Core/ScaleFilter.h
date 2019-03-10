#pragma once

#include "stdafx.h"
#include "DefaultVideoFilter.h"

class ScaleFilter
{
private:
	static bool _hqxInitDone;
	uint32_t _filterScale;
	ScaleFilterType _scaleFilterType;
	uint32_t *_outputBuffer = nullptr;
	uint32_t _width = 0;
	uint32_t _height = 0;

	void ApplyPrescaleFilter(uint32_t *inputArgbBuffer);
	void UpdateOutputBuffer(uint32_t width, uint32_t height);

public:
	ScaleFilter(ScaleFilterType scaleFilterType, uint32_t scale);
	~ScaleFilter();

	uint32_t GetScale();
	uint32_t* ApplyFilter(uint32_t *inputArgbBuffer, uint32_t width, uint32_t height, double scanlineIntensity);
	FrameInfo GetFrameInfo(FrameInfo baseFrameInfo);

	static shared_ptr<ScaleFilter> GetScaleFilter(VideoFilterType filter);
};