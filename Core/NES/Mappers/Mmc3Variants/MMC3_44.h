#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_44 : public MMC3
{
private:
	uint8_t _selectedBlock = 0;

protected:
	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_selectedBlock);
	}

	void Reset(bool softReset) override
	{
		_selectedBlock = 0;
		UpdateState();
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		page &= _selectedBlock <= 5 ? 0x7F : 0xFF;
		page |= _selectedBlock * 0x80;

		MMC3::SelectChrPage(slot, page, memoryType);
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		page &= _selectedBlock <= 5 ? 0x0F : 0x1F;
		page |= _selectedBlock * 0x10;

		MMC3::SelectPrgPage(slot, page, memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if((addr & 0xE001) == 0xA001) {
			_selectedBlock = value & 0x07;
			if(_selectedBlock == 7) {
				_selectedBlock = 6;
			}
		}
		MMC3::WriteRegister(addr, value);
	}
};