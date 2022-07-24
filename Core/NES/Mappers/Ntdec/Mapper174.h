#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

//NTDec 5-in-1 cart - untested, based on Wiki description
class Mapper174 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		WriteRegister(0x8000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint8_t prgBank = (addr >> 4) & 0x07;
		if(addr & 0x80) {
			SelectPrgPage2x(0, prgBank & 0xFE);
		} else {
			SelectPRGPage(0, prgBank);
			SelectPRGPage(1, prgBank);
		}
		SelectCHRPage(0, (addr >> 1) & 0x07);

		SetMirroringType(addr & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};
