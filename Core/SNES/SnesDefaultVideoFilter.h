#pragma once

#include "pch.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Shared/SettingTypes.h"

class SnesDefaultVideoFilter : public BaseVideoFilter
{
private:
	uint32_t _calculatedPalette[0x8000] = {};
	VideoConfig _videoConfig = {};

	bool _blendHighRes = false;
	bool _forceFixedRes = false;

	void InitLookupTable();

	__forceinline static uint32_t BlendPixels(uint32_t a, uint32_t b);
	__forceinline uint32_t GetPixel(uint16_t* ppuFrame, uint32_t offset);

protected:
	void OnBeforeApplyFilter() override;
	FrameInfo GetFrameInfo() override;

public:
	SnesDefaultVideoFilter(Emulator* emu);
	
	void ApplyFilter(uint16_t *ppuOutputBuffer) override;
	OverscanDimensions GetOverscan() override;
};