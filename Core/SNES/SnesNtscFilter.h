#pragma once
#include "pch.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Utilities/NTSC/snes_ntsc.h"

class Emulator;

class SnesNtscFilter : public BaseVideoFilter
{
private:
	snes_ntsc_setup_t _ntscSetup = {};
	snes_ntsc_t _ntscData = {};
	uint32_t* _ntscBuffer = nullptr;

protected:
	void OnBeforeApplyFilter() override;

public:
	SnesNtscFilter(Emulator* emu);
	virtual ~SnesNtscFilter();

	void ApplyFilter(uint16_t *ppuOutputBuffer) override;
	FrameInfo GetFrameInfo() override;
	OverscanDimensions GetOverscan() override;
	HudScaleFactors GetScaleFactor() override;
};