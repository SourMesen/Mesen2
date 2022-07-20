#pragma once
#include "stdafx.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/GbMemoryManager.h"
#include "Utilities/Serializer.h"

class GbMbc3 : public GbCart
{
private:
	bool _hasRtcTimer = false;

	bool _ramRtcEnabled = false;
	uint8_t _prgBank = 1;
	uint8_t _ramBank = 0;
	uint8_t _rtcRegisters[5] = {};

public:
	GbMbc3(bool hasRtcTimer)
	{
		_hasRtcTimer = hasRtcTimer;
	}

	void InitCart() override
	{
		_memoryManager->MapRegisters(0x0000, 0x7FFF, RegisterAccess::Write);
	}

	void RefreshMappings() override
	{
		constexpr int prgBankSize = 0x4000;

		Map(0x0000, 0x3FFF, GbMemoryType::PrgRom, 0, true);
		Map(0x4000, 0x7FFF, GbMemoryType::PrgRom, _prgBank * prgBankSize, true);

		if(_ramRtcEnabled && _ramBank <= 3) {
			Map(0xA000, 0xBFFF, GbMemoryType::CartRam, _ramBank, false);
			_memoryManager->MapRegisters(0xA000, 0xBFFF, RegisterAccess::None);
		} else if(_hasRtcTimer && _ramRtcEnabled && _ramBank >= 0x08 && _ramBank <= 0x0C) {
			_memoryManager->MapRegisters(0xA000, 0xBFFF, RegisterAccess::ReadWrite);
		} else {
			Unmap(0xA000, 0xBFFF);
			_memoryManager->MapRegisters(0xA000, 0xBFFF, RegisterAccess::Read);
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if(_ramRtcEnabled) {
			//Disabled RAM/RTC registers returns 0xFF on reads (?)
			return 0xFF;
		}
		
		//RTC register read (TODO)
		switch(_ramBank) {
			case 0x08: return _rtcRegisters[0]; //Seconds
			case 0x09: return _rtcRegisters[1]; //Minutes
			case 0x0A: return _rtcRegisters[2]; //Hours
			case 0x0B: return _rtcRegisters[3]; //Day counter
			case 0x0C: return _rtcRegisters[4]; //Day counter (upper bit) + carry/halt flags
		}

		//Not reached
		return 0xFF;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr <= 0x7FFF) {
			switch(addr & 0x6000) {
				case 0x0000: _ramRtcEnabled = ((value & 0x0F) == 0x0A); break;
				case 0x2000: _prgBank = std::max<uint8_t>(1, value); break;
				case 0x4000: _ramBank = value & 0x0F; break;
				case 0x6000:
					//RTC register read latch
					break;
			}
			RefreshMappings();
		} else if(addr >= 0xA000 && addr <= 0xBFFF) {
			//RTC register write (TODO)
			switch(_ramBank) {
				case 0x08: _rtcRegisters[0] = value; break; //Seconds
				case 0x09: _rtcRegisters[1] = value; break; //Minutes
				case 0x0A: _rtcRegisters[2] = value; break; //Hours
				case 0x0B: _rtcRegisters[3] = value; break; //Day counter
				case 0x0C: _rtcRegisters[4] = value; break; //Day counter (upper bit) + carry/halt flags
			}
		}
	}

	void Serialize(Serializer& s) override
	{
		SV(_ramRtcEnabled);
		SV(_prgBank);
		SV(_ramBank);
		SVArray(_rtcRegisters, 5);
	}
};