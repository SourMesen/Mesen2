#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class UNROM : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		//First and last PRG page
		SelectPrgPage(0, 0);
		SelectPrgPage(1, -1);

		SelectChrPage(0, 0);
	}

	bool HasBusConflicts() override { return _romInfo.SubMapperID == 2; }

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPrgPage(0, value);
	}
};