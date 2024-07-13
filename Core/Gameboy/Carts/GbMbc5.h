#pragma once
#include "pch.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/GbMemoryManager.h"
#include "Utilities/Serializer.h"
#include "Shared/KeyManager.h"

class GbMbc5 : public GbCart
{
private:
	bool _hasRumble = false;

	bool _ramEnabled = false;
	uint16_t _prgBank = 1;
	uint8_t _ramBank = 0;

public:
	GbMbc5(bool hasRumble)
	{
		_hasRumble = hasRumble;
	}

	void InitCart() override
	{
		_memoryManager->MapRegisters(0x0000, 0x5FFF, RegisterAccess::Write);
	}

	void RefreshMappings() override
	{
		constexpr int prgBankSize = 0x4000;
		constexpr int ramBankSize = 0x2000;

		Map(0x0000, 0x3FFF, GbMemoryType::PrgRom, 0, true);
		Map(0x4000, 0x7FFF, GbMemoryType::PrgRom, _prgBank * prgBankSize, true);

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
		switch(addr & 0x7000) {
			case 0x0000:
			case 0x1000:
				_ramEnabled = (value == 0x0A);
				break;
			
			case 0x2000: 
				_prgBank = (value & 0xFF) | (_prgBank & 0x100);
				break;

			case 0x3000:
				_prgBank = (_prgBank & 0xFF) | ((value & 0x01) << 8);
				break;

			case 0x4000:
			case 0x5000:
				if(_hasRumble) {
					_ramBank = value & 0x07;
					KeyManager::SetForceFeedback((value & 0x08) ? 0xFFFF : 0);
				} else {
					_ramBank = value & 0x0F;
				}
				break;
		}
		RefreshMappings();
	}

	void Serialize(Serializer& s) override
	{
		SV(_ramEnabled);
		SV(_prgBank);
		SV(_ramBank);
	}
};