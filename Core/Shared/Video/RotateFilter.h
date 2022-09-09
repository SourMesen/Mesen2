#pragma once
#include "pch.h"
#include "Shared/SettingTypes.h"

class RotateFilter
{
private:
	uint32_t* _outputBuffer = nullptr;
	uint32_t _angle = 0;
	uint32_t _width = 0;
	uint32_t _height = 0;

	void UpdateOutputBuffer(uint32_t width, uint32_t height);

public:
	RotateFilter(uint32_t angle);
	~RotateFilter();

	uint32_t GetAngle();
	uint32_t* ApplyFilter(uint32_t* inputArgbBuffer, uint32_t width, uint32_t height);
	FrameInfo GetFrameInfo(FrameInfo baseFrameInfo);
};