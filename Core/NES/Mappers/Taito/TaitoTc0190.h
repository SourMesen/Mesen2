#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class TaitoTc0190 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x0400; }

	void InitMapper() override
	{
		SelectPRGPage(2, -2);
		SelectPRGPage(3, -1);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xA003) {
			case 0x8000:
				SelectPRGPage(0, value & 0x3F);
				SetMirroringType((value & 0x40) == 0x40 ? MirroringType::Horizontal : MirroringType::Vertical);
				break;
			case 0x8001:
				SelectPRGPage(1, value & 0x3F);
				break;
			case 0x8002:
				SelectCHRPage(0, value * 2);
				SelectCHRPage(1, value * 2 + 1);
				break;
			case 0x8003:
				SelectCHRPage(2, value * 2);
				SelectCHRPage(3, value * 2 + 1);
				break;
			case 0xA000: case 0xA001: case 0xA002: case 0xA003:
				SelectCHRPage(4 + (addr & 0x03), value);
				break;
		}
	}
};