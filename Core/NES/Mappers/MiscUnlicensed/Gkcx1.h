#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Gkcx1 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x8000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectCHRPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPRGPage(0, (addr >> 3) & 0x03);
		SelectCHRPage(0, addr & 0x07);
	}
};