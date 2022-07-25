#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Mapper203 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectPRGPage(1, 0);
		SelectCHRPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPRGPage(0, value >> 2);
		SelectPRGPage(1, value >> 2);
		SelectCHRPage(0, value & 0x03);
	}
};