#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class ActionEnterprises : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		WriteRegister(0x8000, 0);
	}

	virtual void Reset(bool softReset) override
	{
		WriteRegister(0x8000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint8_t chipSelect = (addr >> 11) & 0x03;

		if(chipSelect == 3) {
			chipSelect = 2;
		}

		uint8_t prgPage = ((addr >> 6) & 0x1F) | (chipSelect << 5);
		if(addr & 0x20) {
			SelectPrgPage(0, prgPage);
			SelectPrgPage(1, prgPage);
		} else {
			SelectPrgPage(0, prgPage & 0xFE);
			SelectPrgPage(1, (prgPage & 0xFE) + 1);
		}

		SelectChrPage(0, ((addr & 0x0F) << 2) | (value & 0x03));

		SetMirroringType(addr & 0x2000 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};
