#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Unl255in1 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x8000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		WriteRegister(0x8000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectCHRPage(0, addr & 0x07);
		SelectPRGPage(0, (addr >> 2) & 0x03);
	}
};