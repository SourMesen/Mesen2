#pragma once
#include "SMS/Carts/SmsCart.h"
#include "SMS/SmsTypes.h"
#include "SMS/SmsMemoryManager.h"
#include "Utilities/Serializer.h"

class SmsSegaCart : public SmsCart
{
private:
	uint8_t _prgBanks[3] = { 0,1,2 };
	bool _ramEnabled[2] = {};
	uint8_t _ramBank = 0;
	
	//TODOSMS unused
	uint8_t _bankShift = 0;
	bool _romWriteEnabled = false;

public:
	using SmsCart::SmsCart;

	void RefreshMappings() override
	{
		MapRegisters(0xFFFC, 0xFFFF, SmsRegisterAccess::Write);

		Map(0x0000, 0x3FFF, MemoryType::SmsPrgRom, _prgBanks[0] * 0x4000, true);

		//First 1kb is fixed
		Map(0x0000, 0x400, MemoryType::SmsPrgRom, 0, true);

		Map(0x4000, 0x7FFF, MemoryType::SmsPrgRom, _prgBanks[1] * 0x4000, true);
		if(_ramEnabled[0]) {
			Map(0x8000, 0xBFFF, MemoryType::SmsCartRam, _ramBank * 0x4000, false);
		} else {
			Map(0x8000, 0xBFFF, MemoryType::SmsPrgRom, _prgBanks[2] * 0x4000, true);
		}

		if(_ramEnabled[1]) {
			Map(0xC000, 0xFFFF, MemoryType::SmsCartRam, (_ramBank ^ 1) * 0x4000, false);
		} else {
			Map(0xC000, 0xFFFF, MemoryType::SmsWorkRam, 0, false);
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		return 0;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr) {
			case 0xFFFC:
				_ramEnabled[0] = (value & 0x08) != 0;
				_ramEnabled[1] = (value & 0x10) != 0;
				_ramBank = (value & 0x04) >> 2;
				_romWriteEnabled = (value & 0x80) != 0;
				_bankShift = (value & 0x03);
				_memoryManager->RefreshMappings();
				break;

			case 0xFFFD: _prgBanks[0] = value; _memoryManager->RefreshMappings(); break;
			case 0xFFFE: _prgBanks[1] = value; _memoryManager->RefreshMappings(); break;
			case 0xFFFF: _prgBanks[2] = value; _memoryManager->RefreshMappings(); break;
		}
	}

	void Serialize(Serializer& s) override
	{
		SVArray(_prgBanks, 3);
		SVArray(_ramEnabled, 2);
		SV(_ramBank);
		SV(_bankShift);
		SV(_romWriteEnabled);
	}
};