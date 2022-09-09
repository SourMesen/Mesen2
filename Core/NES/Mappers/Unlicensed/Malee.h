#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Malee : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x800; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage4x(0, 0);
		SelectPrgPage4x(1, 4);
		SelectPrgPage4x(2, 8);
		SelectPrgPage4x(3, 12);

		SelectChrPage(0, 0);

		SetCpuMemoryMapping(0x6000, 0x67FF, 16, PrgMemoryType::PrgRom);
	}
};