#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class MagicKidGooGoo : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x800; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectPrgPage(1, 0);
		SelectChrPage4x(0, 0);
		SetMirroringType(MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr >= 0x8000 && addr <= 0x9FFF) {
			SelectPrgPage(0, value & 0x07);
		} else if(addr >= 0xC000 && addr <= 0xDFFF) {
			SelectPrgPage(0, (value & 0x07) | 0x08);
		} else if((addr & 0xA000) == 0xA000) {
			SelectChrPage(addr & 0x03, value);
		}
	}
};