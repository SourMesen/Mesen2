#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class MagicKidGooGoo : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x800; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectPRGPage(1, 0);
		SelectChrPage4x(0, 0);
		SetMirroringType(MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr >= 0x8000 && addr <= 0x9FFF) {
			SelectPRGPage(0, value & 0x07);
		} else if(addr >= 0xC000 && addr <= 0xDFFF) {
			SelectPRGPage(0, (value & 0x07) | 0x08);
		} else if((addr & 0xA000) == 0xA000) {
			SelectCHRPage(addr & 0x03, value);
		}
	}
};