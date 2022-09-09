#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Kaiser7013B : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectPrgPage(1, -1);
		SelectChrPage(0, 0);
		SetMirroringType(MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			SelectPrgPage(0, value);
		} else {
			SetMirroringType(value & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
		}
	}
};