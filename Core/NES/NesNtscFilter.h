#pragma once
#include "pch.h"
#include "NES/NesTypes.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Utilities/NTSC/nes_ntsc.h"

class Emulator;

class NesNtscFilter : public BaseVideoFilter
{
private:
	nes_ntsc_setup_t _ntscSetup = {};
	nes_ntsc_t _ntscData = {};
	uint32_t* _ntscBuffer = nullptr;
	PpuModel _ppuModel = PpuModel::Ppu2C02;
	uint8_t _palette[512 * 3] = {};
	NesConfig _nesConfig = {};

protected:
	void OnBeforeApplyFilter() override;
	FrameInfo GetFrameInfo() override;

public:
	NesNtscFilter(Emulator* emu);
	virtual ~NesNtscFilter();
	
	OverscanDimensions GetOverscan() override;
	HudScaleFactors GetScaleFactor() override;

	void ApplyFilter(uint16_t *ppuOutputBuffer) override;
};