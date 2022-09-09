#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class NROM : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectPrgPage(1, 1);

		SelectChrPage(0, 0);
	}
};