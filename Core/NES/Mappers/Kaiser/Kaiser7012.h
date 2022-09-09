#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Kaiser7012 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, 1);
		SelectChrPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr) {
			case 0xE0A0: SelectPrgPage(0, 0); break;
			case 0xEE36: SelectPrgPage(0, 1); break;
		}
	}
};