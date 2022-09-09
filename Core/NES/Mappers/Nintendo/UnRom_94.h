#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class UnRom_94 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		//First and last PRG page
		SelectPrgPage(0, 0);
		SelectPrgPage(1, -1);

		SelectChrPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPrgPage(0, (value >> 2) & 0x07);
	}
};