#pragma once
#include "stdafx.h"
#include "Shared/Video/BaseVideoFilter.h"
#include "PCE/PceConstants.h"
#include "Utilities/NTSC/snes_ntsc.h"

class Emulator;

class PceNtscFilter : public BaseVideoFilter
{
private:
	snes_ntsc_setup_t _ntscSetup;
	snes_ntsc_t _ntscData;
	uint32_t* _ntscBuffer;
	uint16_t* _rgb555Buffer;

protected:
	void OnBeforeApplyFilter();

public:
	PceNtscFilter(Emulator* emu);
	virtual ~PceNtscFilter();

	virtual void ApplyFilter(uint16_t *ppuOutputBuffer);
	virtual FrameInfo GetFrameInfo();
};