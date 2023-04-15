#pragma once
#include "pch.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/GbMemoryManager.h"
#include "Utilities/Serializer.h"

class GbHuc1 : public GbCart
{
private:
	bool _irEnabled = false;
	bool _irOutputEnabled = false;

	uint8_t _prgBank = 1;
	uint8_t _ramBank = 0;

public:
	void InitCart() override
	{
		_memoryManager->MapRegisters(0x0000, 0x7FFF, RegisterAccess::Write);
	}

	void RefreshMappings() override
	{
		Map(0x0000, 0x3FFF, GbMemoryType::PrgRom, 0, true);
		Map(0x4000, 0x7FFF, GbMemoryType::PrgRom, _prgBank * 0x4000, true);

		if(_irEnabled) {
			Unmap(0xA000, 0xBFFF);
			_memoryManager->MapRegisters(0xA000, 0xBFFF, RegisterAccess::ReadWrite);
		} else {
			Map(0xA000, 0xBFFF, GbMemoryType::CartRam, _ramBank * 0x2000, false);
			_memoryManager->MapRegisters(0xA000, 0xBFFF, RegisterAccess::None);
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		//IR sensor (0xC0: no light, 0xC1: light detected)
		return 0xC0;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xE000) {
			case 0x0000: _irEnabled = ((value & 0x0F) == 0x0E); break;
			case 0x2000: _prgBank = value & 0x3F; break;
			case 0x4000: _ramBank = value & 0x03; break;
			case 0x6000: break;
			case 0xA000: _irOutputEnabled = (value & 0x01) != 0; break;
		}
		RefreshMappings();
	}

	void Serialize(Serializer& s) override
	{
		SV(_irEnabled);
		SV(_irOutputEnabled);
		SV(_prgBank);
		SV(_ramBank);
	}
};