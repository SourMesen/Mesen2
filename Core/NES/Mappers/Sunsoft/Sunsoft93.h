#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Sunsoft93 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPRGPage(1, -1);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPRGPage(0, (value >> 4) & 0x07);
		if((value & 0x01) == 0x01) {
			SelectCHRPage(0, 0);
		} else {
			RemovePpuMemoryMapping(0x0000, 0x1FFF);
		}
	}
};