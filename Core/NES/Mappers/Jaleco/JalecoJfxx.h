#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class JalecoJfxx : public BaseMapper
{
private:
	bool _orderedBits = false;

protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0x7FFF; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectChrPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(_orderedBits) {
			//Mapper 101
			SelectChrPage(0, value);
		} else {
			//Mapper 87
			SelectChrPage(0, ((value & 0x01) << 1) | ((value & 0x02) >> 1));
		}
	}

public:
	JalecoJfxx(bool orderedBits) : _orderedBits(orderedBits)
	{
	}
};