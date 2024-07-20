#pragma once
#include "pch.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/GbMemoryManager.h"
#include "Utilities/Serializer.h"

class GbMmm01 : public GbCart
{
private:
	bool _ramEnabled = false;
	uint8_t _prgBank = 0;
	uint8_t _ramBank = 0;
	bool _locked = false;
	uint8_t _outerPrgBank = 0;

public:
	GbMmm01()
	{
	}

	void InitCart() override
	{
		_memoryManager->MapRegisters(0x0000, 0x7FFF, RegisterAccess::Write);
	}

	void RefreshMappings() override
	{
		constexpr int prgBankSize = 0x4000;
		constexpr int ramBankSize = 0x2000;

		if(_locked) {
			Map(0x0000, 0x3FFF, GbMemoryType::PrgRom, _outerPrgBank * prgBankSize, true);
			Map(0x4000, 0x7FFF, GbMemoryType::PrgRom, (_outerPrgBank + _prgBank) * prgBankSize, true);
		} else {
			Map(0x0000, 0x7FFF, GbMemoryType::PrgRom, _gameboy->DebugGetMemorySize(MemoryType::GbPrgRom) - 0x8000, true);
		}

		if(_ramEnabled) {
			Map(0xA000, 0xBFFF, GbMemoryType::CartRam, _ramBank * ramBankSize, false);
			_memoryManager->MapRegisters(0xA000, 0xBFFF, RegisterAccess::None);
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
		switch(addr & 0x6000) {
			case 0x0000: 
				if(_locked) {
					_ramEnabled = ((value & 0x0F) == 0x0A);
				} else {
					_locked = true;
				}
				break;

			case 0x2000:
				if(_locked) {
					_prgBank = value;
				} else {
					_outerPrgBank = value;
				}
				break;

			case 0x4000:
				if(_locked) {
					_ramBank = value;
				}
				break;

			case 0x6000:
				//?
				break;
		}

		RefreshMappings();
	}

	void Serialize(Serializer& s) override
	{
		SV(_ramEnabled);
		SV(_prgBank);
		SV(_ramBank);
		SV(_outerPrgBank);
		SV(_locked);
	}
};