#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

//NTDec 5-in-1 cart - untested, based on Wiki description
class Mapper174 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

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
			SelectPrgPage(0, prgBank);
			SelectPrgPage(1, prgBank);
		}
		SelectChrPage(0, (addr >> 1) & 0x07);

		SetMirroringType(addr & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};
