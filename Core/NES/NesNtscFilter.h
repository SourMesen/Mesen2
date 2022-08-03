#pragma once
#include "stdafx.h"
#include "NES/NesTypes.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "Utilities/NTSC/nes_ntsc.h"

class Emulator;

class NesNtscFilter : public BaseVideoFilter
{
private:
	nes_ntsc_setup_t _ntscSetup;
	nes_ntsc_t _ntscData;
	uint32_t* _ntscBuffer;
	PpuModel _ppuModel = PpuModel::Ppu2C02;
	uint8_t _palette[512 * 3];
	NesConfig _nesConfig;

protected:
	void OnBeforeApplyFilter();
	virtual FrameInfo GetFrameInfo();

public:
	NesNtscFilter(Emulator* emu);
	OverscanDimensions GetOverscan() override;
	virtual ~NesNtscFilter();

	virtual void ApplyFilter(uint16_t *ppuOutputBuffer);
};