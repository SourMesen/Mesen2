#pragma once
#include "stdafx.h"
#include "BaseVideoFilter.h"
#include "../Utilities/snes_ntsc.h"

class Console;

class NtscFilter : public BaseVideoFilter
{
private:
	snes_ntsc_setup_t _ntscSetup;
	snes_ntsc_t _ntscData;
	uint32_t* _ntscBuffer;

protected:
	void OnBeforeApplyFilter();

public:
	NtscFilter(shared_ptr<Console> console);
	virtual ~NtscFilter();

	virtual void ApplyFilter(uint16_t *ppuOutputBuffer);
	virtual FrameInfo GetFrameInfo();
};