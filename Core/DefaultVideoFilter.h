#pragma once

#include "stdafx.h"
#include "BaseVideoFilter.h"

class DefaultVideoFilter : public BaseVideoFilter
{
private:
	double _yiqToRgbMatrix[6];
	uint32_t _calculatedPalette[512];
	PictureSettings _pictureSettings;
	bool _needToProcess = false;

	void InitConversionMatrix(double hueShift, double saturationShift);

	void RgbToYiq(double r, double g, double b, double &y, double &i, double &q);
	void YiqToRgb(double y, double i, double q, double &r, double &g, double &b);

protected:
	void DecodePpuBuffer(uint16_t *ppuOutputBuffer, uint32_t* outputBuffer, bool displayScanlines);
	uint32_t ApplyScanlineEffect(uint16_t ppuPixel, uint8_t scanlineIntensity);
	void OnBeforeApplyFilter();

public:
	DefaultVideoFilter(shared_ptr<Console> console);
	void ApplyFilter(uint16_t *ppuOutputBuffer);
	FrameInfo GetFrameInfo();
};