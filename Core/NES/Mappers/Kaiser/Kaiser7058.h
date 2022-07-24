#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Kaiser7058 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x8000; }
	uint16_t GetCHRPageSize() override { return 0x1000; }
	uint16_t RegisterStartAddress() override { return 0xF000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xF080) {
			case 0xF000: SelectCHRPage(0, value); break;
			case 0xF080: SelectCHRPage(1, value); break;
		}
	}
};