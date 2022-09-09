#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class BmcK3046 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectPrgPage(1, 7);
		SelectChrPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint8_t inner = value & 0x07;
		uint8_t outer = value & 0x38;

		SelectPrgPage(0, outer | inner);
		SelectPrgPage(1, outer | 7);
	}
};
