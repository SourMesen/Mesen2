#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class JalecoJfxx : public BaseMapper
{
private:
	bool _orderedBits = false;

protected:
	uint16_t GetPRGPageSize() override { return 0x8000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0x7FFF; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectCHRPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(_orderedBits) {
			//Mapper 101
			SelectCHRPage(0, value);
		} else {
			//Mapper 87
			SelectCHRPage(0, ((value & 0x01) << 1) | ((value & 0x02) >> 1));
		}
	}

public:
	JalecoJfxx(bool orderedBits) : _orderedBits(orderedBits)
	{
	}
};