#pragma once
#include "pch.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/GbMemoryManager.h"
#include "Utilities/Serializer.h"

class GbMbc1 : public GbCart
{
private:
	bool _ramEnabled = false;
	uint8_t _prgBank = 1;
	uint8_t _ramBank = 0;
	bool _mode = false;
	bool _isMbc1m = false;

public:
	GbMbc1(bool isMbc1m)
	{
		_isMbc1m = isMbc1m;
	}

	void InitCart() override
	{
		_memoryManager->MapRegisters(0x0000, 0x7FFF, RegisterAccess::Write);
	}

	void RefreshMappings() override
	{
		constexpr int prgBankSize = 0x4000;
		constexpr int ramBankSize = 0x2000;

		int shift = _isMbc1m ? 4 : 5;
		int mask = _isMbc1m ? 0x0F : 0x1F;
		Map(0x0000, 0x3FFF, GbMemoryType::PrgRom, (_mode ? (_ramBank << shift) : 0) * prgBankSize, true);

		uint8_t prgBank = (_prgBank & mask) | (_ramBank << shift);
		Map(0x4000, 0x7FFF, GbMemoryType::PrgRom, prgBank*prgBankSize, true);

		if(_ramEnabled) {
			uint8_t ramBank = _mode ? _ramBank : 0;
			Map(0xA000, 0xBFFF, GbMemoryType::CartRam, ramBank*ramBankSize, false);
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
			case 0x0000: _ramEnabled = ((value & 0x0F) == 0x0A); break;
			case 0x2000: _prgBank = std::max(1, value & 0x1F); break;
			case 0x4000: _ramBank = value & 0x03; break;
			case 0x6000: _mode = (value & 0x01) != 0; break;
		}
		RefreshMappings();
	}

	void Serialize(Serializer& s) override
	{
		SV(_ramEnabled); SV(_prgBank); SV(_ramBank); SV(_mode);
	}
};