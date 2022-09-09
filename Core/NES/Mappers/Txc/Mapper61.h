#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper61 : public BaseMapper
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
		uint8_t prgPage = ((addr & 0x0F) << 1) | ((addr >> 5) & 0x01);
		if(addr & 0x10) {
			SelectPrgPage(0, prgPage);
			SelectPrgPage(1, prgPage);
		} else {
			SelectPrgPage(0, prgPage & 0xFE);
			SelectPrgPage(1, (prgPage & 0xFE) + 1);
		}

		SetMirroringType(addr & 0x80 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};
