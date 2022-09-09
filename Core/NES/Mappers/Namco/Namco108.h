#pragma once

#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class Namco108 : public MMC3
{
protected:
	void UpdateMirroring() override
	{
		//Do nothing - Namco 108 has hardwired mirroring only
		//"Mirroring is hardwired, one game uses 4-screen mirroring (Gauntlet, DRROM)."
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		//Redirect all 0x8000-0xFFFF writes to 0x8000-0x8001, all other features do not exist in this version
		addr &= 0x8001;

		if(addr == 0x8000) {
			//Disable CHR Mode 1 and PRG Mode 1
			//"PRG always has the last two 8KiB banks fixed to the end."
			//"CHR always gives the left pattern table (0000-0FFF) the two 2KiB banks, and the right pattern table (1000-1FFF) the four 1KiB banks."
			value &= 0x3F;
		}

		MMC3::WriteRegister(addr, value);
	}
};