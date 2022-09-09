#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_205 : public MMC3
{
private:
	uint8_t _selectedBlock = 0;

protected:
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_selectedBlock);
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		if(_selectedBlock >= 2) {
			page &= 0x7F;
			page |= 0x100;
		}
		if(_selectedBlock == 1 || _selectedBlock == 3) {
			page |= 0x80;
		}

		MMC3::SelectChrPage(slot, page, memoryType);
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		page &= _selectedBlock <= 1 ? 0x1F : 0x0F;
		page |= (_selectedBlock * 0x10);

		MMC3::SelectPrgPage(slot, page, memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_selectedBlock = value & 0x03;
			UpdateState();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};