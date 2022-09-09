#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper225 : public BaseMapper
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
		uint8_t highBit = (addr >> 8) & 0x40;
		uint8_t prgPage = ((addr >> 6) & 0x3F) | highBit;
		if(addr & 0x1000) {
			SelectPrgPage(0, prgPage);
			SelectPrgPage(1, prgPage);
		} else {
			SelectPrgPage(0, prgPage & 0xFE);
			SelectPrgPage(1, (prgPage & 0xFE) + 1);
		}

		SelectChrPage(0, (addr & 0x3F) | highBit);

		SetMirroringType(addr & 0x2000 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};
