#pragma once
#include "pch.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/GbMemoryManager.h"
#include "Utilities/Serializer.h"

class GbMbc7 : public GbCart
{
private:
	bool _ramEnabled = false;
	bool _ramEnabled2 = false;
	uint16_t _prgBank = 1;

	uint16_t _xAccel = 0x8000;
	uint16_t _yAccel = 0x8000;
	bool _latched = false;

public:
	void InitCart() override
	{
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
			case 0x20: return _xAccel & 0xFF;
			case 0x30: return (_xAccel >> 8) & 0xFF;
			case 0x40: return _yAccel & 0xFF;
			case 0x50: return (_yAccel >> 8) & 0xFF;

			//Always 0
			case 0x60: return 0;

			//TODO EEPROM
			case 0x80: return 0x01;

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
				//Reset accelerometer latch
				case 0x00:
					if(value == 0x55) {
						_xAccel = 0x8000;
						_yAccel = 0x8000;
						_latched = false;
					}
					break;

				//Latch accelerometer value
				case 0x10:
					if(!_latched && value == 0xAA) {
						//TODO accelerometer
						_xAccel = 0x81D0;
						_yAccel = 0x81D0;
						_latched = true;
					}
					break;

				case 0x80:
					//TODO EEPROM
					break;
			}
		}
	}

	void Serialize(Serializer& s) override
	{
		SV(_ramEnabled);
		SV(_ramEnabled2);
		SV(_prgBank);
		SV(_xAccel);
		SV(_yAccel);
		SV(_latched);
	}
};