#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper213 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		WriteRegister(0x8000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectChrPage(0, (addr >> 3) & 0x07);
		SelectPrgPage(0, (addr >> 1) & 0x03);
	}
};