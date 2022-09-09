#pragma once

#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_182 : public MMC3
{
protected:
	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xE001) {
			case 0x8001: MMC3::WriteRegister(0xA000, value); break;
			case 0xA000: {
				uint8_t data = (value & 0xF8);
				switch(value & 0x07) {
					case 0: data |= 0; break;
					case 1: data |= 3; break;
					case 2: data |= 1; break;
					case 3: data |= 5; break;
					case 4: data |= 6; break;
					case 5: data |= 7; break;
					case 6: data |= 2; break;
					case 7: data |= 4; break;
				}

				MMC3::WriteRegister(0x8000, data);
				break;
			}
				
			case 0xC000: MMC3::WriteRegister(0x8001, value); break;
			case 0xC001: 
				MMC3::WriteRegister(0xC000, value); 
				MMC3::WriteRegister(0xC001, value);
				break;
			case 0xE000: MMC3::WriteRegister(0xE000, value); break;
			case 0xE001: MMC3::WriteRegister(0xE001, value); break;
		}
	}
};