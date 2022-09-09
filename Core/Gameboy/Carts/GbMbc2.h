#pragma once
#include "pch.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/GbMemoryManager.h"
#include "Utilities/Serializer.h"

class GbMbc2 : public GbCart
{
private:
	bool _ramEnabled = false;
	uint8_t _prgBank = 1;

public:
	void InitCart() override
	{
		_memoryManager->MapRegisters(0x0000, 0x3FFF, RegisterAccess::Write);
		
		for(int i = 0; i < 512; i++) {
			//Ensure cart RAM contains $F in the upper nibble, no matter the contents of save ram
			_cartRam[i] |= 0xF0;
		}
	}

	void RefreshMappings() override
	{
		constexpr int prgBankSize = 0x4000;

		Map(0x0000, 0x3FFF, GbMemoryType::PrgRom, 0, true);

		Map(0x4000, 0x7FFF, GbMemoryType::PrgRom, _prgBank * prgBankSize, true);

		if(_ramEnabled) {
			for(int i = 0; i < 16; i++) {
				Map(0xA000+0x200*i, 0xA1FF+0x200*i, GbMemoryType::CartRam, 0, false);
			}
			_memoryManager->MapRegisters(0xA000, 0xBFFF, RegisterAccess::Write);
		} else {
			Unmap(0xA000, 0xBFFF);
			_memoryManager->MapRegisters(0xA000, 0xBFFF, RegisterAccess::Read);
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		//Disabled RAM returns 0xFF on reads
		return 0xFF;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr >= 0xA000 && addr <= 0xBFFF) {
			//Cut off the top 4 bits for all cart ram writes
			//Set top nibble to $F to mimic open bus
			_cartRam[addr & 0x1FF] = (value & 0x0F) | 0xF0;
		} else {
			switch(addr & 0x100) {
				case 0x000: _ramEnabled = ((value & 0x0F) == 0x0A); break;
				case 0x100: _prgBank = std::max(1, value & 0x0F); break;
			}
			RefreshMappings();
		}
	}

	void Serialize(Serializer& s) override
	{
		SV(_ramEnabled);
		SV(_prgBank);
	}
};