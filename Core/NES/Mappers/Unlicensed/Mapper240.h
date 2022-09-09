#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper240 : public BaseMapper
{
protected:
	uint16_t RegisterStartAddress() override { return 0x4020; }
	uint16_t RegisterEndAddress() override { return 0x5FFF; }
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectChrPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPrgPage(0, (value >> 4) & 0x0F);
		SelectChrPage(0, value & 0x0F);
	}
};