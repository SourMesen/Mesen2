#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Bmc255 : public BaseMapper
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
		uint8_t prgBit = (addr & 0x1000) ? 0 : 1;
		uint8_t bank = ((addr >> 8) & 0x40) | ((addr >> 6) & 0x3F);

		SelectPrgPage(0, bank & ~prgBit);
		SelectPrgPage(1, bank | prgBit);
		SelectChrPage(0, ((addr >> 8) & 0x40) | (addr & 0x3F));
		SetMirroringType(addr & 0x2000 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};