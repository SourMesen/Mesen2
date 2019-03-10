#include "stdafx.h"
#include "DefaultVideoFilter.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include "DebugHud.h"
#include "Console.h"
#include "EmuSettings.h"
#include "SettingTypes.h"

DefaultVideoFilter::DefaultVideoFilter(shared_ptr<Console> console) : BaseVideoFilter(console)
{
	InitLookupTable();
}

void DefaultVideoFilter::InitConversionMatrix(double hueShift, double saturationShift)
{
	double hue = hueShift * M_PI;
	double sat = saturationShift + 1;

	double baseValues[6] = { 0.956f, 0.621f, -0.272f, -0.647f, -1.105f, 1.702f };

	double s = sin(hue) * sat;
	double c = cos(hue) * sat;

	double *output = _yiqToRgbMatrix;
	double *input = baseValues;
	for(int n = 0; n < 3; n++) {
		double i = *input++;
		double q = *input++;
		*output++ = i * c - q * s;
		*output++ = i * s + q * c;
	}
}

void DefaultVideoFilter::InitLookupTable()
{
	VideoConfig config = _console->GetSettings()->GetVideoConfig();

	InitConversionMatrix(config.Hue, config.Saturation);

	double y, i, q;
	for(int rgb555 = 0; rgb555 < 0x8000; rgb555++) {
		double redChannel = To8Bit(rgb555 & 0x1F) / 255.0;
		double greenChannel = To8Bit((rgb555 >> 5) & 0x1F) / 255.0;
		double blueChannel = To8Bit(rgb555 >> 10) / 255.0;

		//Apply brightness, contrast, hue & saturation
		RgbToYiq(redChannel, greenChannel, blueChannel, y, i, q);
		y *= config.Contrast * 0.5f + 1;
		y += config.Brightness * 0.5f;
		YiqToRgb(y, i, q, redChannel, greenChannel, blueChannel);

		int r = std::min(255, (int)(redChannel * 255));
		int g = std::min(255, (int)(greenChannel * 255));
		int b = std::min(255, (int)(blueChannel * 255));

		_calculatedPalette[rgb555] = 0xFF000000 | (r << 16) | (g << 8) | b;
	}

	_videoConfig = config;
}

void DefaultVideoFilter::OnBeforeApplyFilter()
{
	VideoConfig config = _console->GetSettings()->GetVideoConfig();
	if(_videoConfig.Hue != config.Hue || _videoConfig.Saturation != config.Saturation || _videoConfig.Contrast != config.Contrast || _videoConfig.Brightness != config.Brightness) {
		InitLookupTable();
	}
}

uint8_t DefaultVideoFilter::To8Bit(uint8_t color)
{
	return (color << 3) + (color >> 2);
}

uint32_t DefaultVideoFilter::ToArgb(uint16_t rgb555)
{
	uint8_t b = To8Bit(rgb555 >> 10);
	uint8_t g = To8Bit((rgb555 >> 5) & 0x1F);
	uint8_t r = To8Bit(rgb555 & 0x1F);

	return 0xFF000000 | (r << 16) | (g << 8) | b;
}

void DefaultVideoFilter::ApplyFilter(uint16_t *ppuOutputBuffer)
{
	uint32_t *out = GetOutputBuffer();
	FrameInfo frameInfo = GetFrameInfo();

	uint8_t scanlineIntensity = (uint8_t)((1.0 - _console->GetSettings()->GetVideoConfig().ScanlineIntensity) * 255);
	if(scanlineIntensity < 255) {
		for(uint32_t i = 0; i < frameInfo.Height; i++) {
			if(i & 0x01) {
				for(uint32_t j = 0; j < frameInfo.Width; j++) {
					*out = ApplyScanlineEffect(_calculatedPalette[ppuOutputBuffer[i * 512 + j]], scanlineIntensity);
					out++;
				}
			} else {
				for(uint32_t j = 0; j < frameInfo.Width; j++) {
					*out = _calculatedPalette[ppuOutputBuffer[i * 512 + j]];
					out++;
				}
			}
		}
	} else {
		uint32_t pixelCount = frameInfo.Width * frameInfo.Height;
		for(uint32_t i = 0; i < pixelCount; i++) {
			out[i] = _calculatedPalette[ppuOutputBuffer[i]];
		}
	}
}

void DefaultVideoFilter::RgbToYiq(double r, double g, double b, double &y, double &i, double &q)
{
	y = r * 0.299f + g * 0.587f + b * 0.114f;
	i = r * 0.596f - g * 0.275f - b * 0.321f;
	q = r * 0.212f - g * 0.523f + b * 0.311f;
}

void DefaultVideoFilter::YiqToRgb(double y, double i, double q, double &r, double &g, double &b)
{
	r = std::max(0.0, std::min(1.0, (y + _yiqToRgbMatrix[0] * i + _yiqToRgbMatrix[1] * q)));
	g = std::max(0.0, std::min(1.0, (y + _yiqToRgbMatrix[2] * i + _yiqToRgbMatrix[3] * q)));
	b = std::max(0.0, std::min(1.0, (y + _yiqToRgbMatrix[4] * i + _yiqToRgbMatrix[5] * q)));
}