#pragma once

#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_189 : public MMC3
{
private:
	uint8_t _prgReg = 0;

	uint16_t RegisterStartAddress() override { return 0x4120; }
	
	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr <= 0x7FFF) {
			_prgReg = value;
			UpdateState();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}

	void UpdateState() override
	{
		MMC3::UpdateState();

		//"$4120-7FFF:  [AAAA BBBB]"
		//" 'A' and 'B' bits of the $4120 reg seem to be effectively OR'd."
		uint8_t prgPage = (((_prgReg) | (_prgReg >> 4)) & 0x07) * 4;
		SelectPrgPage(0, prgPage);
		SelectPrgPage(1, prgPage+1);
		SelectPrgPage(2, prgPage+2);
		SelectPrgPage(3, prgPage+3);
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_prgReg);
	}
};