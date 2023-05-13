#pragma once
#include "pch.h"
#include "Gameboy/Carts/GbCart.h"
#include "Gameboy/Carts/GbMbc3Rtc.h"
#include "Gameboy/GbMemoryManager.h"
#include "Utilities/Serializer.h"

class GbMbc3 : public GbCart
{
private:
	bool _isMbc30 = false;
	bool _hasRtcTimer = false;

	bool _ramRtcEnabled = false;
	uint8_t _prgBank = 1;
	uint8_t _ramBank = 0;
	GbMbc3Rtc _rtc;

public:
	GbMbc3(Emulator* emu, bool hasRtcTimer, bool isMbc30) : _rtc(emu)
	{
		_isMbc30 = isMbc30;
		_hasRtcTimer = hasRtcTimer;
	}

	void InitCart() override
	{
		_memoryManager->MapRegisters(0x0000, 0x7FFF, RegisterAccess::Write);
	}

	void RefreshMappings() override
	{
		constexpr int prgBankSize = 0x4000;
		constexpr int ramBankSize = 0x2000;

		Map(0x0000, 0x3FFF, GbMemoryType::PrgRom, 0, true);
		Map(0x4000, 0x7FFF, GbMemoryType::PrgRom, _prgBank * prgBankSize, true);

		if(_ramRtcEnabled && _ramBank <= (_isMbc30 ? 7 : 3)) {
			Map(0xA000, 0xBFFF, GbMemoryType::CartRam, _ramBank * ramBankSize, false);
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
		if(!_ramRtcEnabled) {
			//Disabled RAM/RTC registers returns 0xFF on reads (?)
			return 0xFF;
		}
		
		if(_hasRtcTimer) {
			return _rtc.Read(_ramBank & 0x0F);
		} else {
			return 0xFF;
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr <= 0x7FFF) {
			switch(addr & 0x6000) {
				case 0x0000: _ramRtcEnabled = ((value & 0x0F) == 0x0A); break;
				case 0x2000: _prgBank = std::max<uint8_t>(1, value); break;
				case 0x4000: _ramBank = value & 0x0F; break;
				case 0x6000: _rtc.LatchData(); break;
			}
			RefreshMappings();
		} else if(addr >= 0xA000 && addr <= 0xBFFF) {
			_rtc.Write(_ramBank & 0x0F, value);
		}
	}

	void Serialize(Serializer& s) override
	{
		SV(_ramRtcEnabled);
		SV(_prgBank);
		SV(_ramBank);
		SV(_rtc);
	}
};