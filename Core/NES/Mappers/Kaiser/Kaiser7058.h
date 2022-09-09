#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Kaiser7058 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x1000; }
	uint16_t RegisterStartAddress() override { return 0xF000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xF080) {
			case 0xF000: SelectChrPage(0, value); break;
			case 0xF080: SelectChrPage(1, value); break;
		}
	}
};