#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class GxRom : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x8000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPRGPage(0, GetPowerOnByte() & 0x03);
		SelectCHRPage(0, GetPowerOnByte() & 0x03);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPRGPage(0, (value >> 4) & 0x03);
		SelectCHRPage(0, value & 0x03);
	}
};