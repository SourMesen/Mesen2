#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Bmc190in1 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		WriteRegister(0x8000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPrgPage(0, (value >> 2) & 0x07);
		SelectPrgPage(1, (value >> 2) & 0x07);
		SelectChrPage(0, (value >> 2) & 0x07);
		SetMirroringType(value & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};