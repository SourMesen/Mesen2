#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_Kof97 : public MMC3
{
protected:
	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		value = (value & 0xD8) | ((value & 0x20) >> 4) | ((value & 0x04) << 3) | ((value & 0x02) >> 1) | ((value & 0x01) << 2);
		if(addr == 0x9000) {
			addr = 0x8001;
		} else if(addr == 0xD000) {
			addr = 0xC001;
		} else if(addr == 0xF000) {
			addr = 0xE001;
		}

		MMC3::WriteRegister(addr, value);
	}
};