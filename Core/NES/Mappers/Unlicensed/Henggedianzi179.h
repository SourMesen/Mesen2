#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Henggedianzi179 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x8000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		AddRegisterRange(0x5000, 0x5FFF, MemoryOperation::Write);
		SelectPrgPage(0, 0);
		SelectChrPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr >= 0x8000) {
			SetMirroringType(value & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
		} else {
			SelectPrgPage(0, value >> 1);
		}
	}
};