#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class BmcNtd03 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
	}

	void Reset(bool softReset) override
	{
		BaseMapper::Reset(softReset);
		WriteRegister(0x8000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint8_t prg = ((addr >> 10) & 0x1E);
		uint8_t chr= ((addr & 0x0300) >> 5) | (addr & 0x07);
		if(addr & 0x80) {
			SelectPrgPage(0, prg | ((addr >> 6) & 1));
			SelectPrgPage(1, prg | ((addr >> 6) & 1));
		} else {
			SelectPrgPage2x(0, prg & 0xFE);
		}

		SelectChrPage(0, chr);
		SetMirroringType(addr & 0x400 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};