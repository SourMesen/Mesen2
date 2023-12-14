#pragma once
#include "pch.h"
#include "Shared/MemoryType.h"
#include "Utilities/ISerializable.h"

enum class SmsRegisterAccess;
class SmsMemoryManager;

class SmsCart : public ISerializable
{
protected:
	SmsMemoryManager* _memoryManager = nullptr;

	void Map(uint16_t start, uint16_t end, MemoryType type, uint32_t offset, bool readonly);
	void Unmap(uint16_t start, uint16_t end);
	void MapRegisters(uint16_t start, uint16_t end, SmsRegisterAccess access);

public:
	SmsCart(SmsMemoryManager* memoryManager);
	virtual ~SmsCart();

	virtual void RefreshMappings();
	virtual uint8_t ReadRegister(uint16_t addr);
	virtual uint8_t PeekRegister(uint16_t addr);
	virtual void WriteRegister(uint16_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};