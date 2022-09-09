#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper246 : public BaseMapper
{
protected:
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0x67FF; }
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x0800; }

	void InitMapper() override
	{
		SelectPrgPage(3, 0xFF);
	}

	virtual void Reset(bool softReset) override
	{
		SelectPrgPage(3, 0xFF);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if((addr & 0x7) <= 0x3) {
			SelectPrgPage(addr & 0x03, value);
		} else {
			SelectChrPage(addr & 0x03, value);
		}
	}
};
