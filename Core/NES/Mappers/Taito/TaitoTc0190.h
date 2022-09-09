#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class TaitoTc0190 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x0400; }

	void InitMapper() override
	{
		SelectPrgPage(2, -2);
		SelectPrgPage(3, -1);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xA003) {
			case 0x8000:
				SelectPrgPage(0, value & 0x3F);
				SetMirroringType((value & 0x40) == 0x40 ? MirroringType::Horizontal : MirroringType::Vertical);
				break;
			case 0x8001:
				SelectPrgPage(1, value & 0x3F);
				break;
			case 0x8002:
				SelectChrPage(0, value * 2);
				SelectChrPage(1, value * 2 + 1);
				break;
			case 0x8003:
				SelectChrPage(2, value * 2);
				SelectChrPage(3, value * 2 + 1);
				break;
			case 0xA000: case 0xA001: case 0xA002: case 0xA003:
				SelectChrPage(4 + (addr & 0x03), value);
				break;
		}
	}
};