#pragma once
#include "pch.h"

class ScanlineFilter
{
private:
	static uint32_t ApplyScanlineEffect(uint32_t argb, uint8_t scanlineIntensity)
	{
		uint8_t r = ((argb & 0xFF0000) >> 16) * scanlineIntensity / 255;
		uint8_t g = ((argb & 0xFF00) >> 8) * scanlineIntensity / 255;
		uint8_t b = (argb & 0xFF) * scanlineIntensity / 255;

		return 0xFF000000 | (r << 16) | (g << 8) | b;
	}

public:
	static void ApplyFilter(uint32_t* buffer, uint32_t width, uint32_t height, double scanlineIntensity, uint8_t scale)
	{
		if(scanlineIntensity <= 0) {
			return;
		}

		scale = std::max<uint8_t>(2, scale);
		int linesToSkip = scale - 1;

		uint8_t intensity = (uint8_t)((1.0 - scanlineIntensity) * 255);

		for(uint32_t i = 0, len = height / scale; i < len; i++) {
			buffer += width * linesToSkip;
			
			for(uint32_t j = 0; j < width; j++) {
				*buffer = ApplyScanlineEffect(*buffer, intensity);
				buffer++;
			}
		}
	}
};