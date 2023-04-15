#pragma once
#include "pch.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/GbMemoryManager.h"
#include "Utilities/Serializer.h"

class GbMbc6 : public GbCart
{
private:
	bool _ramEnabled = false;
	bool _flashEnabled = false;
	bool _flashWriteEnabled = false;
	bool _flashBankEnabled[2] = {};
	uint8_t _prgBanks[2] = {};
	uint8_t _ramBanks[2] = {};

public:
	void InitCart() override
	{
		_memoryManager->MapRegisters(0x0000, 0x3FFF, RegisterAccess::Write);
	}

	void RefreshMappings() override
	{
		Map(0x0000, 0x3FFF, GbMemoryType::PrgRom, 0, true);

		if(_flashBankEnabled[0]) {
			_memoryManager->MapRegisters(0x4000, 0x5FFF, RegisterAccess::ReadWrite);
		} else {
			Map(0x4000, 0x5FFF, GbMemoryType::PrgRom, _prgBanks[0] * 0x2000, true);
			_memoryManager->MapRegisters(0x4000, 0x5FFF, RegisterAccess::None);
		}

		if(_flashBankEnabled[1]) {
			_memoryManager->MapRegisters(0x6000, 0x7FFF, RegisterAccess::ReadWrite);
		} else {
			Map(0x6000, 0x7FFF, GbMemoryType::PrgRom, _prgBanks[1] * 0x2000, true);
			_memoryManager->MapRegisters(0x6000, 0x7FFF, RegisterAccess::None);
		}

		if(_ramEnabled) {
			Map(0xA000, 0xAFFF, GbMemoryType::CartRam, _ramBanks[0] * 0x1000, false);
			Map(0xB000, 0xBFFF, GbMemoryType::CartRam, _ramBanks[1] * 0x1000, false);
		} else {
			Unmap(0xA000, 0xBFFF);
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		//TODO - Flash reads
		return 0xFF;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x4000) {
			switch(addr & 0x3C00) {
				case 0x0000: _ramEnabled = (value == 0x0A); break;
				case 0x0400: _ramBanks[0] = value & 0x07; break;
				case 0x0800: _ramBanks[1] = value & 0x07; break;

				case 0x0C00:
					if(_flashWriteEnabled) {
						_flashEnabled = (value & 0x01) != 0;
					}
					break;

				case 0x1000: _flashWriteEnabled = (value & 0x01) != 0; break;

				case 0x2000:
				case 0x2400:
					_prgBanks[0] = value & 0x7F;
					break;

				case 0x2800:
				case 0x2C00:
					_flashBankEnabled[0] = (value == 0x08);
					break;

				case 0x3000:
				case 0x3400:
					_prgBanks[1] = value & 0x7F;
					break;

				case 0x3800:
				case 0x3C00:
					_flashBankEnabled[1] = (value == 0x08);
					break;
			}
			RefreshMappings();
		} else {
			//TODO - Flash writes
		}
	}

	void Serialize(Serializer& s) override
	{
		SV(_ramEnabled);
		SV(_prgBanks[0]);
		SV(_prgBanks[1]);
		SV(_ramBanks[0]);
		SV(_ramBanks[1]);

		SV(_flashEnabled);
		SV(_flashWriteEnabled);
		SV(_flashBankEnabled[0]);
		SV(_flashBankEnabled[1]);
	}
};