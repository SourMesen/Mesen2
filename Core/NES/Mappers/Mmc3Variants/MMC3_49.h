#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_49 : public MMC3
{
private:
	uint8_t _selectedBlock = 0;
	uint8_t _prgReg = 0;
	uint8_t _prgMode = 0;

protected:
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_selectedBlock);
		SV(_prgReg);
		SV(_prgMode);
	}

	void Reset(bool softReset) override
	{
		_prgReg = 0;
		_prgMode = false;
		_selectedBlock = 0;
		UpdateState();
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		page &= 0x7F;
		page |= 0x80 * _selectedBlock;
		MMC3::SelectChrPage(slot, page, memoryType);
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		if(_prgMode) {
			page &= 0x0F;
			page |= 0x10 * _selectedBlock;
		} else {
			page = _prgReg * 4 + slot;
		}
		MMC3::SelectPrgPage(slot, page, memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if(CanWriteToWorkRam()) {
				_selectedBlock = (value >> 6) & 0x03;
				_prgReg = (value >> 4) & 0x03;
				_prgMode = value & 0x01;
				UpdateState();
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};