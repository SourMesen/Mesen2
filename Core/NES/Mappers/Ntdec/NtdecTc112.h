#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class NtdecTc112 : public BaseMapper
{
protected:
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0x7FFF; }

	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x0800; }

	void InitMapper() override
	{
		SelectPRGPage(1, -3);
		SelectPRGPage(2, -2);
		SelectPRGPage(3, -1);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0x03){
			case 0: 
				SelectCHRPage(0, value >> 1);
				SelectCHRPage(1, (value >> 1) + 1);
				break;

			case 1:
				SelectCHRPage(2, value >> 1);
				break;

			case 2:
				SelectCHRPage(3, value >> 1);
				break;

			case 3:
				SelectPRGPage(0, value);
				break;
		}
	}
};
