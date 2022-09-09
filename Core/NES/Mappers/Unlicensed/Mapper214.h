#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper214 : public BaseMapper
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
		SelectChrPage(0, addr & 0x03);
		SelectPrgPage(0, (addr >> 2) & 0x03);
		SelectPrgPage(1, (addr >> 2) & 0x03);
	}
};