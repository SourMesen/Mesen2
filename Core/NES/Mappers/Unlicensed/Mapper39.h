#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

//Used by Study and Game 32-in-1 (Ch)
class Mapper39 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x8000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectCHRPage(0, 0);
	}

	void Reset(bool softReset) override
	{
		SelectPRGPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPRGPage(0, value);
	}
};
