#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

//Mostly taken from FCEUX's code
class MMC3_Coolboy : public MMC3
{
private:
	uint8_t _exRegs[4] = {};

protected:
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint32_t GetChrRamSize() override { return 0x40000; }

	void Reset(bool softReset) override
	{
		memset(_exRegs, 0, sizeof(_exRegs));		
		BaseMapper::Reset(softReset);
		MMC3::ResetMmc3();

		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SVArray(_exRegs, 4);
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		uint16_t addr = slot * 0x400;
		uint32_t mask = 0xFF ^ (_exRegs[0] & 0x80);
		int cbase = _chrMode ? 0x1000 : 0;
		if(_exRegs[3] & 0x10) {
			if(_exRegs[3] & 0x40) {				
				switch(cbase ^ addr) {
					case 0x0400:
					case 0x0C00: page &= 0x7F; break;
				}
			}
			
			MMC3::SelectChrPage(slot,
				(page & 0x80 & mask) | ((((_exRegs[0] & 0x08) << 4) & ~mask)) 
				| ((_exRegs[2] & 0x0F) << 3)
				| slot
			);
		} else {
			if(_exRegs[3] & 0x40) {
				switch(cbase ^ addr) {
					case 0x0000: page = _registers[0]; break;
					case 0x0800: page = _registers[1]; break;
					case 0x0400:
					case 0x0C00: page = 0; break;
				}
			}

			MMC3::SelectChrPage(slot, (page & mask) | (((_exRegs[0] & 0x08) << 4) & ~mask));
		}
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		uint16_t addr = 0x8000 + slot * 0x2000;
		uint32_t mask = ((0x3F | (_exRegs[1] & 0x40) | ((_exRegs[1] & 0x20) << 2)) ^ ((_exRegs[0] & 0x40) >> 2)) ^ ((_exRegs[1] & 0x80) >> 2);
		uint32_t base = ((_exRegs[0] & 0x07) >> 0) | ((_exRegs[1] & 0x10) >> 1) | ((_exRegs[1] & 0x0C) << 2) | ((_exRegs[0] & 0x30) << 2);

		if((_exRegs[3] & 0x40) && (page >= 0xFE) && _prgMode) {
			switch(slot) {
				case 1: if(_prgMode) page = 0; break;
				case 2: if(!_prgMode) page = 0; break;
				case 3: page = 0; break;
			}
		}

		if(!(_exRegs[3] & 0x10)) {
			MMC3::SelectPrgPage(slot, (((base << 4) & ~mask)) | (page & mask));
		} else {
			mask &= 0xF0;
			uint8_t emask;
			if((((_exRegs[1] & 0x02) != 0))) {
				emask = (_exRegs[3] & 0x0C) | ((addr & 0x4000) >> 13);
			} else {
				emask = _exRegs[3] & 0x0E;
			}

			MMC3::SelectPrgPage(slot, ((base << 4) & ~mask) | (page & mask) | emask | (slot & 0x01));
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if(GetState().RegA001 & 0x80) {
				WritePrgRam(addr, value);
			}

			if((_exRegs[3] & 0x90) != 0x80) {
				_exRegs[addr & 0x03] = value;
				UpdateState();
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};