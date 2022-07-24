#pragma once
#include "NES/BaseMapper.h"

class Sachen_143 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	uint16_t RegisterStartAddress() override { return 0x4100; }
	uint16_t RegisterEndAddress() override { return 0x5FFF; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectPRGPage(1, 1);

		SelectCHRPage(0, 0);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		return (~addr & 0x3F) | 0x40;
	}
};