#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_126 : public MMC3
{
private:
	uint8_t _exRegs[4] = {};

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		uint16_t reg = _exRegs[0];
		page &= ((~reg >> 2) & 0x10) | 0x0F;
		page |= (reg & (0x06 | (reg & 0x40) >> 6)) << 4 | (reg & 0x10) << 3;

		if(!(_exRegs[3] & 0x03)) {
			MMC3::SelectPrgPage(slot, page, memoryType);
		} else if((_prgMode << 1) == slot) {
			if((_exRegs[3] & 0x03) == 0x03) {
				MMC3::SelectPrgPage(0, page, memoryType);
				MMC3::SelectPrgPage(1, page + 1, memoryType);
				MMC3::SelectPrgPage(2, page + 2, memoryType);
				MMC3::SelectPrgPage(3, page + 3, memoryType);
			} else {
				MMC3::SelectPrgPage(0, page, memoryType);
				MMC3::SelectPrgPage(1, page + 1, memoryType);
				MMC3::SelectPrgPage(2, page, memoryType);
				MMC3::SelectPrgPage(3, page + 1, memoryType);
			}
		}
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType) override
	{
		if(!(_exRegs[3] & 0x10)) {
			MMC3::SelectChrPage(slot, GetChrOuterBank() | (page & ((_exRegs[0] & 0x80) - 1)));
		}
	}

	uint16_t GetChrOuterBank()
	{
		uint16_t reg = _exRegs[0];
		return 
			(~reg << 0 & 0x0080 & _exRegs[2]) |
			(reg << 4 & 0x0080 & reg) |
			(reg << 3 & 0x0100) |
			(reg << 5 & 0x0200);
	}

	void InitMapper() override
	{
		MMC3::InitMapper();
		memset(_exRegs, 0, sizeof(_exRegs));
		AddRegisterRange(0x6000, 0x8000, MemoryOperation::Write);
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SVArray(_exRegs, 4);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			addr &= 0x03;

			if(addr == 0x01 || addr == 0x02 || ((addr == 0x00 || addr == 0x03) && !(_exRegs[3] & 0x80))) {
				if(_exRegs[addr] != value) {
					_exRegs[addr] = value;

					if(_exRegs[3] & 0x10) {
						uint16_t page = GetChrOuterBank() | ((_exRegs[2] & 0x0F) << 3);
						for(int i = 0; i < 8; i++) {
							MMC3::SelectChrPage(i, page + i);
						}
					} else {
						MMC3::UpdateChrMapping();
					}

					MMC3::UpdatePrgMapping();
				}
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}		
	}
};