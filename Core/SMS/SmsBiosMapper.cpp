#include "pch.h"
#include "SMS/SmsBiosMapper.h"
#include "SMS/SmsMemoryManager.h"
#include "Utilities/Serializer.h"

SmsBiosMapper::SmsBiosMapper(SmsMemoryManager* memoryManager)
{
	_memoryManager = memoryManager;
}

void SmsBiosMapper::WriteRegister(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0xFFFD: _prgBanks[0] = value; _memoryManager->RefreshMappings(); break;
		case 0xFFFE: _prgBanks[1] = value; _memoryManager->RefreshMappings(); break;
		case 0xFFFF: _prgBanks[2] = value; _memoryManager->RefreshMappings(); break;
	}
}

void SmsBiosMapper::RefreshMappings(bool cartridgeEnabled)
{
	_memoryManager->MapRegisters(0xFFFD, 0xFFFF, SmsRegisterAccess::Write);

	if(!cartridgeEnabled) {
		_memoryManager->Map(0x0000, 0x3FFF, MemoryType::SmsBootRom, _prgBanks[0] * 0x4000, true);
	}
	
	//First 1kb is fixed?
	_memoryManager->Map(0x0000, 0x03FF, MemoryType::SmsBootRom, 0, true);

	if(!cartridgeEnabled) {
		_memoryManager->Map(0x4000, 0x7FFF, MemoryType::SmsBootRom, _prgBanks[1] * 0x4000, true);
		_memoryManager->Map(0x8000, 0xBFFF, MemoryType::SmsBootRom, _prgBanks[2] * 0x4000, true);
	}
}

void SmsBiosMapper::Serialize(Serializer& s)
{
	SVArray(_prgBanks, 3);
}
