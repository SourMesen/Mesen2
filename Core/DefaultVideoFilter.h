#pragma once

#include "stdafx.h"
#include "BaseVideoFilter.h"
#include "SettingTypes.h"

class DefaultVideoFilter : public BaseVideoFilter
{
private:
	uint32_t _calculatedPalette[0x8000] = {};
	VideoConfig _videoConfig = {};

	uint16_t* _prevFrame = nullptr;
	bool _gbBlendFrames = false;
	bool _gbcAdjustColors = false;

	void InitLookupTable();

	__forceinline static uint8_t To8Bit(uint8_t color);
	__forceinline static uint32_t BlendPixels(uint32_t a, uint32_t b);
	__forceinline uint32_t GetPixel(uint16_t* ppuFrame, uint32_t offset);

protected:
	void OnBeforeApplyFilter();

public:
	DefaultVideoFilter(shared_ptr<Emulator> emu);
	void ApplyFilter(uint16_t *ppuOutputBuffer);

	static uint32_t ToArgb(uint16_t rgb555);
};