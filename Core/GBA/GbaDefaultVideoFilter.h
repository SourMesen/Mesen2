#pragma once
#include "pch.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Shared/Video/GenericNtscFilter.h"
#include "Shared/SettingTypes.h"

class Emulator;

class GbaDefaultVideoFilter : public BaseVideoFilter
{
private:
	uint32_t _calculatedPalette[0x8000] = {};
	VideoConfig _videoConfig = {};

	uint16_t* _prevFrame = nullptr;
	bool _blendFrames = false;
	bool _gbaAdjustColors = false;

	bool _applyNtscFilter = false;
	GenericNtscFilter _ntscFilter;

	void InitLookupTable();

	__forceinline static uint32_t BlendPixels(uint32_t a, uint32_t b);
	__forceinline uint32_t GetPixel(uint16_t* ppuFrame, uint32_t offset);

protected:
	void OnBeforeApplyFilter() override;
	FrameInfo GetFrameInfo() override;

public:
	GbaDefaultVideoFilter(Emulator* emu, bool applyNtscFilter);
	~GbaDefaultVideoFilter();

	void ApplyFilter(uint16_t* ppuOutputBuffer) override;
};