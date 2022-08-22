#pragma once
#include "stdafx.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "PCE/PceConstants.h"
#include "Utilities/NTSC/snes_ntsc.h"

class Emulator;

class PceNtscFilter : public BaseVideoFilter
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
	
	OverscanDimensions GetOverscan() override;
	void ApplyFilter(uint16_t *ppuOutputBuffer) override;
	FrameInfo GetFrameInfo() override;
};