#pragma once
#include "SMS/Carts/SmsCart.h"
#include "SMS/SmsTypes.h"
#include "SMS/SmsMemoryManager.h"
#include "Utilities/Serializer.h"

class SmsKoreanCart : public SmsCart
{
private:
	uint8_t _prgBank = 0;

public:
	using SmsCart::SmsCart;

	void RefreshMappings() override
	{
		MapRegisters(0xA000, 0xA000, SmsRegisterAccess::Write);

		//First 32kb is fixed
		Map(0x0000, 0x7FFF, MemoryType::SmsPrgRom, 0, true);
		Map(0x8000, 0xBFFF, MemoryType::SmsPrgRom, _prgBank * 0x4000, true);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		return 0;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr) {
			case 0xA000: _prgBank = value; _memoryManager->RefreshMappings(); break;
		}
	}

	void Serialize(Serializer& s) override
	{
		SV(_prgBank);
	}
};