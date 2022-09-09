#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper244 : public BaseMapper
{
private:
	static constexpr uint8_t _lutPrg[4][4] = { 
		{ 0, 1, 2, 3 },
		{ 3, 2, 1, 0 },
		{ 0, 2, 1, 3 },
		{ 3, 1, 2, 0 }
	};

	static constexpr uint8_t _lutChr[8][8] = {
		{ 0, 1, 2, 3, 4, 5, 6, 7 }, 
		{ 0, 2, 1, 3, 4, 6, 5, 7 }, 
		{ 0, 1, 4, 5, 2, 3, 6, 7 },
		{ 0, 4, 1, 5, 2, 6, 3, 7 },
		{ 0, 4, 2, 6, 1, 5, 3, 7 },
		{ 0, 2, 4, 6, 1, 3, 5, 7 },
		{ 7, 6, 5, 4, 3, 2, 1, 0 },
		{ 7, 6, 5, 4, 3, 2, 1, 0 }
	};

protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectChrPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(value & 0x08) {
			SelectChrPage(0, _lutChr[(value >> 4) & 0x07][value & 0x07]);
		} else {
			SelectPrgPage(0, _lutPrg[(value >> 4) & 0x03][value & 0x03]);
		}
	}
};