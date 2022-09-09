#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Sunsoft89 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(1, -1);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPrgPage(0, (value >> 4) & 0x07);
		SelectChrPage(0, (value & 0x07) | ((value & 0x80) >> 4));
		SetMirroringType((value & 0x08) == 0x08 ? MirroringType::ScreenBOnly : MirroringType::ScreenAOnly);
	}
};