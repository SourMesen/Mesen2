#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Cc21 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x8000; }
	uint16_t GetCHRPageSize() override { return 0x1000; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectCHRPage(0, 0);
		SelectCHRPage(1, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint8_t latch = (uint8_t)addr;
		if(addr == 0x8000) {
			latch = value;
		}

		if(_chrRomSize == 0x2000) {
			SelectCHRPage(0, latch & 0x01);
			SelectCHRPage(1, latch & 0x01);
		} else {
			//Overdumped roms
			SelectChrPage2x(0, (latch & 0x01) << 1);
		}

		SetMirroringType(latch & 0x01 ? MirroringType::ScreenBOnly : MirroringType::ScreenAOnly);
	}
};