#pragma once
#include "pch.h"
#include "NES/Mappers/Tengen/Rambo1.h"

class Rambo1_158 : public Rambo1
{
	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if((addr & 0xE001) == 0x8001) {
			uint8_t nametable = value >> 7;

			if(_currentRegister & 0x80) {
				switch(_currentRegister & 0x07) {
					case 2: SetNametable(0, nametable); break;
					case 3: SetNametable(1, nametable); break;
					case 4: SetNametable(2, nametable); break;
					case 5: SetNametable(3, nametable); break;
				}
			} else {
				switch(_currentRegister & 0x07) {
					case 0:
						SetNametable(0, nametable);
						SetNametable(1, nametable);
						break;

					case 1:
						SetNametable(2, nametable);
						SetNametable(3, nametable);
						break;
				}
			}
		}

		if((addr & 0xE001) != 0xA000) {
			Rambo1::WriteRegister(addr, value);
		}
	}
};