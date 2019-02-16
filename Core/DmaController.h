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
	static constexpr uint8_t _transferByteCount[8] = { 1, 2, 2, 4, 4, 2, 4 };
	static constexpr uint8_t _transferOffset[8][4] = {
		{ 0, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 1, 1 },
		{ 0, 1, 2, 3 }, { 0, 1, 0, 1 }, { 0, 0, 0, 0 }, { 0, 0, 1, 1 }
	};

	DmaChannelConfig _channel[8] = {};
	MemoryManager *_memoryManager;
	
	void RunSingleTransfer(DmaChannelConfig &channel, uint32_t &bytesLeft);
	void RunDma(DmaChannelConfig &channel);

public:
	DmaController(MemoryManager *memoryManager);

	void Write(uint16_t addr, uint8_t value);
};