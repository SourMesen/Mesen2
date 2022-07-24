#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Kaiser7013B : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectPRGPage(1, -1);
		SelectCHRPage(0, 0);
		SetMirroringType(MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			SelectPRGPage(0, value);
		} else {
			SetMirroringType(value & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
		}
	}
};