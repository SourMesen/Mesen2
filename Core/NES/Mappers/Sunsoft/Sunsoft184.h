#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Sunsoft184 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x1000; }
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0x7FFF; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectChrPage(0, value & 0x07);

		//"The most significant bit of H is always set in hardware."
		SelectChrPage(1, 0x80 | ((value >> 4) & 0x07));
	}
};