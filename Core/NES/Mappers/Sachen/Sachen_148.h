#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Sachen_148 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x8000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	bool HasBusConflicts() override { return true; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPRGPage(0, (value >> 3) & 0x01);
		SelectCHRPage(0, value & 0x07);
	}
};