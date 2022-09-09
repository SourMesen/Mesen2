#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Sachen_149 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectChrPage(0, (value >> 7) & 0x01);
	}
};