#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Lh51 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectPRGPage(1, 13);
		SelectPRGPage(2, 14);
		SelectPRGPage(3, 15);
		SelectCHRPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xE000) {
			case 0x8000:
				SelectPRGPage(0, value & 0x0F);
				break;

			case 0xE000:
				SetMirroringType(value & 0x08 ? MirroringType::Horizontal : MirroringType::Vertical);
				break;
		}
	}
};
