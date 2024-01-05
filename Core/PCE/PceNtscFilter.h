#pragma once
#include "pch.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "PCE/PceDefaultVideoFilter.h"
#include "PCE/PceConstants.h"
#include "Utilities/NTSC/snes_ntsc.h"

class Emulator;

class PceNtscFilter : public PceDefaultVideoFilter
{
private:
	snes_ntsc_setup_t _ntscSetup = {};
	snes_ntsc_t _ntscData = {};
	uint32_t* _ntscBuffer = nullptr;
	uint16_t* _rgb555Buffer = nullptr;

protected:
	void OnBeforeApplyFilter() override;

public:
	PceNtscFilter(Emulator* emu);
	virtual ~PceNtscFilter();
	
	void ApplyFilter(uint16_t *ppuOutputBuffer) override;
	FrameInfo GetFrameInfo() override;
	OverscanDimensions GetOverscan() override;
	HudScaleFactors GetScaleFactor() override;
};