#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper204 : public BaseMapper
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
		uint8_t bitMask = addr & 0x06;
		uint8_t page = bitMask + ((bitMask == 0x06) ? 0 : (addr & 0x01));
		SelectPrgPage(0, page);
		SelectPrgPage(1, bitMask + ((bitMask == 0x06) ? 1 : (addr & 0x01)));
		SelectChrPage(0, page);
		SetMirroringType(addr & 0x10 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};