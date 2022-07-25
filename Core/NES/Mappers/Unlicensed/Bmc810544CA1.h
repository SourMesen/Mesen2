#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Bmc810544CA1 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

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
		uint16_t bank = (addr >> 6) & 0xFFFE;
		if(addr & 0x40) {
			SelectPrgPage2x(0, bank);
		} else {
			SelectPRGPage(0, bank | ((addr >> 5) & 0x01));
			SelectPRGPage(1, bank | ((addr >> 5) & 0x01));
		}
		SelectCHRPage(0, addr & 0x0F);
		SetMirroringType(addr & 0x10 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};