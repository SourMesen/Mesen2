#include "pch.h"
#include "SMS/Carts/SmsCart.h"
#include "SMS/SmsTypes.h"
#include "SMS/SmsMemoryManager.h"
#include "Shared/MessageManager.h"
#include "Utilities/Serializer.h"

SmsCart::SmsCart(SmsMemoryManager* memoryManager)
{
	_memoryManager = memoryManager;
}

SmsCart::~SmsCart()
{
}

void SmsCart::Map(uint16_t start, uint16_t end, MemoryType type, uint32_t offset, bool readonly)
{
	_memoryManager->Map(start, end, type, offset, readonly);
}

void SmsCart::Unmap(uint16_t start, uint16_t end)
{
	_memoryManager->Unmap(start, end);
}

void SmsCart::MapRegisters(uint16_t start, uint16_t end, SmsRegisterAccess access)
{
	_memoryManager->MapRegisters(start, end, access);
}

void SmsCart::RefreshMappings()
{
	_memoryManager->Map(0x0000, 0xBFFF, MemoryType::SmsPrgRom, 0, true);
}

uint8_t SmsCart::ReadRegister(uint16_t addr)
{
	LogDebug("[Debug] SMS - Missing read handler: $" + HexUtilities::ToHex(addr));
	return 0;
}

uint8_t SmsCart::PeekRegister(uint16_t addr)
{
	return 0;
}

void SmsCart::WriteRegister(uint16_t addr, uint8_t value)
{
}

void SmsCart::Serialize(Serializer& s)
{
}