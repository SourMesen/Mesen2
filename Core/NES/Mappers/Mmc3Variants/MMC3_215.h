#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

//Unif: UNL-8237
class MMC3_215 : public MMC3
{
protected:
	static constexpr uint8_t _lutReg[8][8] = {
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0, 2, 6, 1, 7, 3, 4, 5 },
		{ 0, 5, 4, 1, 7, 2, 6, 3 },
		{ 0, 6, 3, 7, 5, 2, 4, 1 },
		{ 0, 2, 5, 3, 6, 1, 7, 4 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
	};

	static constexpr uint8_t _lutAddr[8][8] =
	{
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 3, 2, 0, 4, 1, 5, 6, 7 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 5, 0, 1, 2, 3, 7, 6, 4 },
		{ 3, 1, 0, 5, 2, 4, 6, 7 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
		{ 0, 1, 2, 3, 4, 5, 6, 7 },
	};

	uint8_t _exRegs[3] = {};

	uint16_t RegisterStartAddress() override { return 0x5000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		_exRegs[0] = 0;
		_exRegs[1] = 3;
		_exRegs[2] = 0;

		MMC3::InitMapper();
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SVArray(_exRegs, 3);
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType) override
	{
		if(_exRegs[0] & 0x40) {
			MMC3::SelectChrPage(slot, ((_exRegs[1] & 0x0C) << 6) | (page & 0x7F) | ((_exRegs[1] & 0x20) << 2), memoryType);
		} else {
			MMC3::SelectChrPage(slot, ((_exRegs[1] & 0x0C) << 6) | page, memoryType);
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
				bank = ((_exRegs[1] & 0x03) << 4) | (_exRegs[0] & 0x07) | (sbank >> 1);
			}
		} else {
			mask = 0x1F;
			if(_exRegs[0] & 0x80) {
				bank = ((_exRegs[1] & 0x03) << 4) | (_exRegs[0] & 0x0F);
			}
		}

		if(_exRegs[0] & 0x80) {
			bank <<= 1;
			if(_exRegs[0] & 0x20) {
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
			MMC3::SelectPrgPage(slot, ((_exRegs[1] & 0x03) << 5) | (page & mask) | sbank);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			switch(addr) {
				case 0x5000: _exRegs[0] = value; UpdateState(); break;
				case 0x5001: _exRegs[1] = value; UpdateState(); break;
				case 0x5007: _exRegs[2] = value; break;
			}
		} else {
			uint8_t lutValue = _lutAddr[_exRegs[2]][((addr >> 12) & 0x06) | (addr & 0x01)];
			addr = (lutValue & 0x01) | ((lutValue & 0x06) << 12) | 0x8000;
			if(lutValue == 0) {
				value = (value & 0xC0) | (_lutReg[_exRegs[2]][value & 0x07]);
			}
			MMC3::WriteRegister(addr, value);
		}
	}
};