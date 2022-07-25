#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class CpRom : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x8000; }
	uint16_t GetCHRPageSize() override { return 0x1000; }
	uint32_t GetChrRamSize() override { return 0x4000; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectCHRPage(0, 0);
		SetMirroringType(MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr >= 0x8000) {
			SelectCHRPage(1, value & 0x03);
		}
	}
};