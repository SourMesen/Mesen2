#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class NsfCart31 : public BaseMapper
{
protected:
	uint16_t RegisterStartAddress() override { return 0x5000; }
	uint16_t RegisterEndAddress() override { return 0x5FFF; }

	uint16_t GetPrgPageSize() override { return 0x1000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		WriteRegister(0x5FFF, 0xFF);

		SelectChrPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPrgPage(addr & 0x07, value);
	}
};
