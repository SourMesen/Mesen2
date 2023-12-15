#pragma once
#include "SMS/Carts/SmsCart.h"
#include "SMS/SmsTypes.h"
#include "SMS/SmsMemoryManager.h"
#include "Utilities/Serializer.h"

class SmsMsxCart : public SmsCart
{
private:
	uint8_t _prgBanks[4] = {};

public:
	using SmsCart::SmsCart;

	void RefreshMappings() override
	{
		MapRegisters(0x0000, 0x0003, SmsRegisterAccess::Write);

		//First 16kb is fixed
		Map(0x0000, 0x3FFF, MemoryType::SmsPrgRom, 0, true);

		Map(0x4000, 0x5FFF, MemoryType::SmsPrgRom, _prgBanks[0] * 0x2000, true);
		Map(0x6000, 0x7FFF, MemoryType::SmsPrgRom, _prgBanks[1] * 0x2000, true);
		Map(0x8000, 0x9FFF, MemoryType::SmsPrgRom, _prgBanks[2] * 0x2000, true);
		Map(0xA000, 0xBFFF, MemoryType::SmsPrgRom, _prgBanks[3] * 0x2000, true);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		return 0;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr) {
			case 0: _prgBanks[2] = value; _memoryManager->RefreshMappings(); break;
			case 1: _prgBanks[3] = value; _memoryManager->RefreshMappings(); break;
			case 2: _prgBanks[0] = value; _memoryManager->RefreshMappings(); break;
			case 3: _prgBanks[1] = value; _memoryManager->RefreshMappings(); break;
		}
	}

	void Serialize(Serializer& s) override
	{
		SVArray(_prgBanks, 4);
	}
};