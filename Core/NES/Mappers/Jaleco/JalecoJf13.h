#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class JalecoJf13 : public BaseMapper
{
protected:
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0x7FFF; }
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0x7000) {
			case 0x6000:
				SelectPrgPage(0, (value & 0x30) >> 4);
				SelectChrPage(0, (value & 0x03) | ((value >> 4) & 0x04));
				break;

			case 0x7000:
				//Audio not supported
				break;
		}
		
	}
};