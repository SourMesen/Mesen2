#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper58 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectPrgPage(1, 1);
		SelectChrPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint8_t prgBank = addr & 0x07;
		if(addr & 0x40) {
			SelectPrgPage(0, prgBank);
			SelectPrgPage(1, prgBank);
		} else {
			SelectPrgPage2x(0, prgBank & 0x06);
		}
		SelectChrPage(0, (addr >> 3) & 0x07);

		SetMirroringType(addr & 0x80 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};
