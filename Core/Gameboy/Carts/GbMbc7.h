#pragma once
#include "pch.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/Carts/Eeprom93Lc56.h"
#include "Gameboy/Carts/GbMbc7Accelerometer.h"
#include "Gameboy/GbMemoryManager.h"
#include "Utilities/Serializer.h"

class GbMbc7 : public GbCart
{
private:
	shared_ptr<GbMbc7Accelerometer> _accelerometer;
	Eeprom93Lc56 _eeprom;
	bool _ramEnabled = false;
	bool _ramEnabled2 = false;
	uint16_t _prgBank = 1;

public:
	void InitCart() override
	{
		_accelerometer.reset(new GbMbc7Accelerometer(_gameboy->GetEmulator()));
		_gameboy->GetControlManager()->AddSystemControlDevice(_accelerometer);

		_eeprom.SetRam(_cartRam);
		_memoryManager->MapRegisters(0x0000, 0x5FFF, RegisterAccess::Write);
	}

	void RefreshMappings() override
	{
		Map(0x0000, 0x3FFF, GbMemoryType::PrgRom, 0, true);
		Map(0x4000, 0x7FFF, GbMemoryType::PrgRom, _prgBank * 0x4000, true);
		_memoryManager->MapRegisters(0xA000, 0xBFFF, _ramEnabled && _ramEnabled2 ? RegisterAccess::ReadWrite : RegisterAccess::None);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if(addr >= 0xB000) {
			//Always returns 0xFF?
			return 0xFF;
		}

		switch(addr & 0xF0) {
			case 0x20:
			case 0x30:
			case 0x40:
			case 0x50:
				return _accelerometer->Read(addr);

			//Always 0
			case 0x60: return 0;

			case 0x80: return _eeprom.Read();

			default: return 0xFF;
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x6000) {
			switch(addr & 0x6000) {
				case 0x0000: _ramEnabled = (value == 0x0A); break;
				case 0x2000: _prgBank = value & 0x7F; break;
				case 0x4000: _ramEnabled2 = (value == 0x40); break;
			}
			RefreshMappings();
		} else {
			switch(addr & 0xF0) {
				case 0x00:
				case 0x10:
					_accelerometer->Write(addr, value);
					break;

				case 0x80:
					_eeprom.Write(value);
					break;
			}
		}
	}

	void Serialize(Serializer& s) override
	{
		SV(_ramEnabled);
		SV(_ramEnabled2);
		SV(_prgBank);
		SV(_eeprom);
		SV(_accelerometer);
	}
};
