#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"

class SmsMemoryManager;

class SmsBiosMapper final : public ISerializable
{
private:
	SmsMemoryManager* _memoryManager = nullptr;
	uint8_t _prgBanks[3] = { 0,1,2 };

public:
	SmsBiosMapper(SmsMemoryManager* memoryManager);

	void WriteRegister(uint16_t addr, uint8_t value);
	void RefreshMappings(bool cartridgeEnabled);

	void Serialize(Serializer& s) override;
};