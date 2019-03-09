#pragma once
#include "stdafx.h"
#include "CpuTypes.h"

class MemoryManager;

struct DmaChannelConfig
{
	bool InvertDirection;
	bool Decrement;
	bool FixedTransfer;
	bool HdmaIndirectAddressing;
	uint8_t TransferMode;

	uint16_t SrcAddress;
	uint8_t SrcBank;

	uint8_t DestAddress;
	uint16_t TransferSize;

	uint8_t HdmaBank;
	uint16_t HdmaTableAddress;
	uint8_t HdmaLineCounterAndRepeat;
	bool DoTransfer;
	bool HdmaFinished;

	bool InterruptedByHdma;
	bool UnusedFlag;
};

class DmaController
{
private:
	static constexpr uint8_t _transferByteCount[8] = { 1, 2, 2, 4, 4, 4, 2, 4 };
	static constexpr uint8_t _transferOffset[8][4] = {
		{ 0, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 1, 1 },
		{ 0, 1, 2, 3 }, { 0, 1, 0, 1 }, { 0, 0, 0, 0 }, { 0, 0, 1, 1 }
	};

	bool _hdmaPending = false;
	uint8_t _hdmaChannels = 0;

	DmaChannelConfig _channel[8] = {};
	MemoryManager *_memoryManager;
	
	void CopyDmaByte(uint32_t addressBusA, uint16_t addressBusB, bool fromBtoA);

	void RunSingleTransfer(DmaChannelConfig &channel);
	void RunDma(DmaChannelConfig &channel);
	void RunHdmaTransfer(DmaChannelConfig &channel);
	
public:
	DmaController(MemoryManager *memoryManager);

	void InitHdmaChannels();
	void ProcessHdmaChannels();

	void Write(uint16_t addr, uint8_t value);
	uint8_t Read(uint16_t addr);
};