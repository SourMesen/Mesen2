#pragma once
#include "stdafx.h"
#include "CpuTypes.h"

class MemoryManager;

struct DmaChannelConfig
{
	bool InvertDirection;
	bool Decrement;
	bool FixedTransfer;
	bool HdmaPointers;
	uint8_t TransferMode;

	uint32_t SrcAddress;
	uint8_t DestAddress;
	uint16_t TransferSize;
};

class DmaController
{
private:
	DmaChannelConfig _channel[8] = {};
	shared_ptr<MemoryManager> _memoryManager;

public:
	DmaController(shared_ptr<MemoryManager> memoryManager);

	void RunDma(DmaChannelConfig & channel);

	void Write(uint16_t addr, uint8_t value);
};