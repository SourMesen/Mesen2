#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"

class WsCpu;
class WsMemoryManager;

class WsCpuPrefetch final : public ISerializable
{
private:
	uint16_t _fetchCs = 0;
	uint16_t _fetchIp = 0;

	uint8_t _readPos = 0;
	uint8_t _writePos = 0;
	uint8_t _size = 0;

	uint8_t _waitCycles = 0;

	uint8_t _data[16] = {};

	WsMemoryManager* _memoryManager = nullptr;
	WsCpu* _cpu = nullptr;

	bool IsFull();
	void PushByte(uint8_t value);

public:
	WsCpuPrefetch(WsCpu* cpu, WsMemoryManager* memoryManager);

	void Prefetch();
	void ProcessRep(uint8_t opCode);
	uint8_t Read();
	void Clear(uint16_t cs, uint16_t ip);

	void Serialize(Serializer& s) override;
};
