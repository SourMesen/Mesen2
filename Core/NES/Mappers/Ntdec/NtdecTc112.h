#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class NtdecTc112 : public BaseMapper
{
protected:
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0x7FFF; }

	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x0800; }

	void InitMapper() override
	{
		SelectPrgPage(1, -3);
		SelectPrgPage(2, -2);
		SelectPrgPage(3, -1);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0x03){
			case 0: 
				SelectChrPage(0, value >> 1);
				SelectChrPage(1, (value >> 1) + 1);
				break;

			case 1:
				SelectChrPage(2, value >> 1);
				break;

			case 2:
				SelectChrPage(3, value >> 1);
				break;

			case 3:
				SelectPrgPage(0, value);
				break;
		}
	}
};
