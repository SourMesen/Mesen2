#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper231 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectPrgPage(1, 0);
		SelectChrPage(0, 0);
	}

	void Reset(bool softReset) override
	{
		SelectPrgPage(0, 0);
		SelectPrgPage(1, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint8_t prgBank = ((addr >> 5) & 0x01) | (addr & 0x1E);
		SelectPrgPage(0, prgBank & 0x1E);
		SelectPrgPage(1, prgBank);
		SetMirroringType(addr & 0x80 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};
