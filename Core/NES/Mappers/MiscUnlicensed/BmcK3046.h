#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class BmcK3046 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectPRGPage(1, 7);
		SelectCHRPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint8_t inner = value & 0x07;
		uint8_t outer = value & 0x38;

		SelectPRGPage(0, outer | inner);
		SelectPRGPage(1, outer | 7);
	}
};
