#pragma once
#include "pch.h"
#include "NES/Mappers/Mmc3Variants/MMC3_215.h"

class Unl8237A : public MMC3_215
{
	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType) override
	{
		if(_exRegs[0] & 0x40) {
			MMC3::SelectChrPage(slot, ((_exRegs[1] & 0x0E) << 7) | (page & 0x7F) | ((_exRegs[1] & 0x20) << 2), memoryType);
		} else {
			MMC3::SelectChrPage(slot, ((_exRegs[1] & 0x0E) << 7) | page, memoryType);
		}
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		uint8_t sbank = 0;
		uint8_t bank = 0;
		uint8_t mask = 0;

		if(_exRegs[0] & 0x40) {
			mask = 0x0F;
			sbank = (_exRegs[1] & 0x10);
			if(_exRegs[0] & 0x80) {
				bank = ((_exRegs[1] & 0x03) << 4) | ((_exRegs[1] & 0x08) << 3) | (_exRegs[0] & 0x07) | (sbank >> 1);
			}
		} else {
			mask = 0x1F;
			if(_exRegs[0] & 0x80) {
				bank = ((_exRegs[1] & 0x03) << 4) | ((_exRegs[1] & 0x08) << 3) | (_exRegs[0] & 0x0F);
			}
		}

		if(_exRegs[0] & 0x80) {
			bank <<= 1;
			if(_exRegs[0] & 0x20) {
				bank &= 0xFC;
				MMC3::SelectPrgPage(0, bank);
				MMC3::SelectPrgPage(1, bank + 1);
				MMC3::SelectPrgPage(2, bank + 2);
				MMC3::SelectPrgPage(3, bank + 3);
			} else {
				MMC3::SelectPrgPage(0, bank);
				MMC3::SelectPrgPage(1, bank + 1);
				MMC3::SelectPrgPage(2, bank);
				MMC3::SelectPrgPage(3, bank + 1);
			}
		} else {
			MMC3::SelectPrgPage(slot, ((_exRegs[1] & 0x03) << 5) | ((_exRegs[1] & 0x08) << 4) | (page & mask) | sbank);
		}
	}
};