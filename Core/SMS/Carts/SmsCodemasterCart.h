#pragma once
#include "SMS/Carts/SmsCart.h"
#include "SMS/SmsTypes.h"
#include "SMS/SmsMemoryManager.h"

class SmsCodemasterCart : public SmsCart
{
private:
	uint8_t _prgBanks[3] = { 0,1,0 };

public:
	using SmsCart::SmsCart;

	void RefreshMappings() override
	{
		MapRegisters(0, 0xBFFF, SmsRegisterAccess::Write);

		Map(0x0000, 0x3FFF, MemoryType::SmsPrgRom, _prgBanks[0] * 0x4000, true);
		Map(0x4000, 0x7FFF, MemoryType::SmsPrgRom, _prgBanks[1] * 0x4000, true);
		if(_prgBanks[1] & 0x80) {
			Map(0x8000, 0x9FFF, MemoryType::SmsPrgRom, _prgBanks[2] * 0x4000, true);
			Map(0xA000, 0xBFFF, MemoryType::SmsCartRam, 0, false);
		} else {
			Map(0x8000, 0xBFFF, MemoryType::SmsPrgRom, _prgBanks[2] * 0x4000, true);
		}
		Map(0xC000, 0xFFFF, MemoryType::SmsWorkRam, 0, false);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		return 0;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xC000) {
			case 0x0000: _prgBanks[0] = value; _memoryManager->RefreshMappings(); break;
			case 0x4000: _prgBanks[1] = value; _memoryManager->RefreshMappings(); break;
			case 0x8000: _prgBanks[2] = value; _memoryManager->RefreshMappings(); break;
		}
	}

	void Serialize(Serializer& s) override
	{
		SVArray(_prgBanks, 3);
	}
};