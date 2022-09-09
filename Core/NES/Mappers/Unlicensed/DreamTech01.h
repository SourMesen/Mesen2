#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class DreamTech01 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x5020; }
	uint16_t RegisterEndAddress() override { return 0x5020; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectPrgPage(1, 8);
		SelectChrPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPrgPage(0, value & 0x07);
	}
};