#pragma once

#include "stdafx.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Shared/SettingTypes.h"

class SnesDefaultVideoFilter : public BaseVideoFilter
{
private:
	uint32_t _calculatedPalette[0x8000] = {};
	VideoConfig _videoConfig = {};

	bool _snesBlendHighRes = false;

	void InitLookupTable();

	__forceinline static uint8_t To8Bit(uint8_t color);
	__forceinline static uint32_t BlendPixels(uint32_t a, uint32_t b);
	__forceinline uint32_t GetPixel(uint16_t* ppuFrame, uint32_t offset);

protected:
	void OnBeforeApplyFilter() override;

public:
	SnesDefaultVideoFilter(Emulator* emu);
	
	FrameInfo GetFrameInfo() override;
	void ApplyFilter(uint16_t *ppuOutputBuffer) override;

	static uint32_t ToArgb(uint16_t rgb555);
};