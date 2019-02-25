#include "stdafx.h"
#include "DefaultVideoFilter.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>
#include "DebugHud.h"
#include "Console.h"

DefaultVideoFilter::DefaultVideoFilter(shared_ptr<Console> console) : BaseVideoFilter(console)
{
	InitConversionMatrix(_pictureSettings.Hue, _pictureSettings.Saturation);
}

void DefaultVideoFilter::InitConversionMatrix(double hueShift, double saturationShift)
{
	_pictureSettings.Hue = hueShift;
	_pictureSettings.Saturation = saturationShift;

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

void DefaultVideoFilter::OnBeforeApplyFilter()
{
	/*PictureSettings currentSettings = _console->GetSettings()->GetPictureSettings();
	if(_pictureSettings.Hue != currentSettings.Hue || _pictureSettings.Saturation != currentSettings.Saturation) {
		InitConversionMatrix(currentSettings.Hue, currentSettings.Saturation);
	}
	_pictureSettings = currentSettings;
	_needToProcess = _pictureSettings.Hue != 0 || _pictureSettings.Saturation != 0 || _pictureSettings.Brightness || _pictureSettings.Contrast;

	if(_needToProcess) {
		double y, i, q;
		uint32_t* originalPalette = _console->GetSettings()->GetRgbPalette();

		for(int pal = 0; pal < 512; pal++) {
			uint32_t pixelOutput = originalPalette[pal];
			double redChannel = ((pixelOutput & 0xFF0000) >> 16) / 255.0;
			double greenChannel = ((pixelOutput & 0xFF00) >> 8) / 255.0;
			double blueChannel = (pixelOutput & 0xFF) / 255.0;

			//Apply brightness, contrast, hue & saturation
			RgbToYiq(redChannel, greenChannel, blueChannel, y, i, q);
			y *= _pictureSettings.Contrast * 0.5f + 1;
			y += _pictureSettings.Brightness * 0.5f;
			YiqToRgb(y, i, q, redChannel, greenChannel, blueChannel);

			int r = std::min(255, (int)(redChannel * 255));
			int g = std::min(255, (int)(greenChannel * 255));
			int b = std::min(255, (int)(blueChannel * 255));

			_calculatedPalette[pal] = 0xFF000000 | (r << 16) | (g << 8) | b;
		}
	} else {
		memcpy(_calculatedPalette, _console->GetSettings()->GetRgbPalette(), sizeof(_calculatedPalette));
	}*/
}

void DefaultVideoFilter::DecodePpuBuffer(uint16_t *ppuOutputBuffer, uint32_t* outputBuffer, bool displayScanlines)
{
	/*uint32_t* out = outputBuffer;
	OverscanDimensions overscan = GetOverscan();
	uint8_t scanlineIntensity = (uint8_t)((1.0 - _console->GetSettings()->GetPictureSettings().ScanlineIntensity) * 255);
	for(uint32_t i = overscan.Top, iMax = 240 - overscan.Bottom; i < iMax; i++) {
		if(displayScanlines && (i + overscan.Top) % 2 == 0) {
			for(uint32_t j = overscan.Left, jMax = 256 - overscan.Right; j < jMax; j++) {
				*out = ApplyScanlineEffect(ppuOutputBuffer[i * 256 + j], scanlineIntensity);
				out++;
			}
		} else {
			for(uint32_t j = overscan.Left, jMax = 256 - overscan.Right; j < jMax; j++) {
				*out = _calculatedPalette[ppuOutputBuffer[i * 256 + j]];
				out++;
			}
		}
	}*/
}

void DefaultVideoFilter::ApplyFilter(uint16_t *ppuOutputBuffer)
{
	uint32_t *out = GetOutputBuffer();
	uint32_t pixelCount = GetFrameInfo().Width * GetFrameInfo().Height;
	for(uint32_t i = 0; i < pixelCount; i++) {
		uint16_t rgb555 = ppuOutputBuffer[i];
		uint8_t b = (rgb555 >> 10) * 256 / 32;
		uint8_t g = ((rgb555 >> 5) & 0x1F) * 256 / 32;
		uint8_t r = (rgb555 & 0x1F) * 256 / 32;

		out[i] = 0xFF000000 | (r << 16) | (g << 8) | b;
	}
	//DecodePpuBuffer(ppuOutputBuffer, GetOutputBuffer(), _console->GetSettings()->GetVideoFilterType() <= VideoFilterType::BisqwitNtsc);
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

uint32_t DefaultVideoFilter::ApplyScanlineEffect(uint16_t ppuPixel, uint8_t scanlineIntensity)
{
	uint32_t pixelOutput = _calculatedPalette[ppuPixel];

	uint8_t r = ((pixelOutput & 0xFF0000) >> 16) * scanlineIntensity / 255;
	uint8_t g = ((pixelOutput & 0xFF00) >> 8) * scanlineIntensity / 255;
	uint8_t b = (pixelOutput & 0xFF) * scanlineIntensity / 255;

	return 0xFF000000 | (r << 16) | (g << 8) | b;
}