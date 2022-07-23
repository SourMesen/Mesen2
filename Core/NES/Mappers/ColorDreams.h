#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class ColorDreams : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x8000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	bool HasBusConflicts() override { return true; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectCHRPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(_romInfo.MapperID == 144) {
			//"This addition means that only the ROM's least significant bit always wins bus conflicts."
			value |= (ReadRam(addr) & 0x01);
		}

		//TODO: Re-add size restriction when adding an option to prevent oversized roms
		SelectPRGPage(0, value & 0x0F);
		SelectCHRPage(0, (value >> 4) & 0x0F);
	}
};