#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Lh51 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectPrgPage(1, 13);
		SelectPrgPage(2, 14);
		SelectPrgPage(3, 15);
		SelectChrPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xE000) {
			case 0x8000:
				SelectPrgPage(0, value & 0x0F);
				break;

			case 0xE000:
				SetMirroringType(value & 0x08 ? MirroringType::Horizontal : MirroringType::Vertical);
				break;
		}
	}
};
