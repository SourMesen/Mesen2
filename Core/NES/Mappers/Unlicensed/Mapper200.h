#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper200 : public BaseMapper
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

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint8_t bank = addr & 0x07;
		SelectPrgPage(0, bank);
		SelectPrgPage(1, bank);
		SelectChrPage(0, bank);

		SetMirroringType(addr & 0x08 ? MirroringType::Vertical : MirroringType::Horizontal);
	}
};
