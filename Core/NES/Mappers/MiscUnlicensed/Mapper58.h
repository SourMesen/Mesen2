#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Mapper58 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectPRGPage(1, 1);
		SelectCHRPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint8_t prgBank = addr & 0x07;
		if(addr & 0x40) {
			SelectPRGPage(0, prgBank);
			SelectPRGPage(1, prgBank);
		} else {
			SelectPrgPage2x(0, prgBank & 0x06);
		}
		SelectCHRPage(0, (addr >> 3) & 0x07);

		SetMirroringType(addr & 0x80 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};
