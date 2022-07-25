#pragma once
#include "NES/BaseMapper.h"

class Sachen_143 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	uint16_t RegisterStartAddress() override { return 0x4100; }
	uint16_t RegisterEndAddress() override { return 0x5FFF; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectPrgPage(1, 1);

		SelectChrPage(0, 0);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		return (~addr & 0x3F) | 0x40;
	}
};