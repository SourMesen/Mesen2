#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Mapper62 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectPRGPage(1, 1);
		SelectCHRPage(0, 0);
	}

	virtual void Reset(bool softReset) override
	{
		if(softReset) {
			SelectPRGPage(0, 0);
			SelectPRGPage(1, 1);
			SelectCHRPage(0, 0);
		}
	}
	
	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint8_t prgPage = ((addr & 0x3F00) >> 8) | (addr & 0x40);
		uint8_t chrPage = ((addr & 0x1F) << 2) | (value & 0x03);
		if(addr & 0x20) {
			SelectPRGPage(0, prgPage);
			SelectPRGPage(1, prgPage);
		} else {
			SelectPRGPage(0, prgPage & 0xFE);
			SelectPRGPage(1, (prgPage & 0xFE) + 1);
		}

		SelectCHRPage(0, chrPage);

		SetMirroringType(addr & 0x80 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};
