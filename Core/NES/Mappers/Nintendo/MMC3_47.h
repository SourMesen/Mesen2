#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_47 : public MMC3
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

	void Reset(bool softReset) override
	{
		_selectedBlock = 0;
		UpdateState();
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		page &= 0x7F;
		if(_selectedBlock == 1) {
			page |= 0x80;
		}

		MMC3::SelectChrPage(slot, page, memoryType);
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		page &= 0x0F;
		if(_selectedBlock == 1) {
			page |= 0x10;
		}

		MMC3::SelectPrgPage(slot, page, memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if(CanWriteToWorkRam()) {
				_selectedBlock = value & 0x01;
				UpdateState();
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};