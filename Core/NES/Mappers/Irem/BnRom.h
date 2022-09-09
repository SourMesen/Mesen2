#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class BnRom : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, GetPowerOnByte());
		SelectChrPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		//"While the original BNROM board connects only 2 bits, it is recommended that emulators implement this as an 8-bit register allowing selection of up to 8 MB PRG ROM if present."
		SelectPrgPage(0, value);
	}
};