#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

//Used by Study and Game 32-in-1 (Ch)
class Mapper39 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectChrPage(0, 0);
	}

	void Reset(bool softReset) override
	{
		SelectPrgPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPrgPage(0, value);
	}
};
