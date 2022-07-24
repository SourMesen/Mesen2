#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class BnRom : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x8000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPRGPage(0, GetPowerOnByte());
		SelectCHRPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		//"While the original BNROM board connects only 2 bits, it is recommended that emulators implement this as an 8-bit register allowing selection of up to 8 MB PRG ROM if present."
		SelectPRGPage(0, value);
	}
};