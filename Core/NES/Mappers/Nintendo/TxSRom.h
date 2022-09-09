#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

//TKSROM and TLSROM
class TxSRom : public MMC3
{
protected:
	void UpdateMirroring() override
	{
		//This is disabled, 8001 writes are used to setup mirroring instead
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if((addr & 0xE001) == 0x8001) {
			uint8_t nametable = value >> 7;
				
			if(GetChrMode() == 0) {
				switch(GetCurrentRegister()) {
					case 0:
						SetNametable(0, nametable);
						SetNametable(1, nametable);
						break;

					case 1:
						SetNametable(2, nametable);
						SetNametable(3, nametable);
						break;
				}
			} else {
				switch(GetCurrentRegister()) {
					case 2: SetNametable(0, nametable); break;
					case 3: SetNametable(1, nametable); break;
					case 4: SetNametable(2, nametable); break;
					case 5: SetNametable(3, nametable); break;
				}
			}
		}
		MMC3::WriteRegister(addr, value);
	}
};