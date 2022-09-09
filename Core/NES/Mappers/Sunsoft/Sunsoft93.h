#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Sunsoft93 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(1, -1);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPrgPage(0, (value >> 4) & 0x07);
		if((value & 0x01) == 0x01) {
			SelectChrPage(0, 0);
		} else {
			RemovePpuMemoryMapping(0x0000, 0x1FFF);
		}
	}
};