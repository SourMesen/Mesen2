#pragma once

#include "stdafx.h"
#include "BaseVideoFilter.h"
#include "SettingTypes.h"

class DefaultVideoFilter : public BaseVideoFilter
{
private:
	uint32_t _calculatedPalette[0x8000];
	double _yiqToRgbMatrix[6];
	VideoConfig _videoConfig;

	void InitConversionMatrix(double hueShift, double saturationShift);
	void InitLookupTable();

	void RgbToYiq(double r, double g, double b, double &y, double &i, double &q);
	void YiqToRgb(double y, double i, double q, double &r, double &g, double &b);
	__forceinline static uint8_t To8Bit(uint8_t color);

protected:
	void OnBeforeApplyFilter();

public:
	DefaultVideoFilter(shared_ptr<Console> console);
	void ApplyFilter(uint16_t *ppuOutputBuffer);

	static uint32_t ToArgb(uint16_t rgb555);
};