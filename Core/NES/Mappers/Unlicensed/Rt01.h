#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Rt01 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x800; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectPrgPage(1, 0);
		SelectChrPage(0, 0);
		SelectChrPage(1, 0);
		SelectChrPage(2, 0);
		SelectChrPage(3, 0);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if((addr >= 0xCE80 && addr < 0xCF00) || (addr >= 0xFE80 && addr < 0xFF00)) {
			return 0xF2 | (rand() & 0x0D);
		} else {
			return InternalReadRam(addr);
		}
	}
};