#pragma once
#include "stdafx.h"
#include "CpuTypes.h"
#include "../Utilities/ISerializable.h"

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

class DmaController final : public ISerializable
{
private:
	bool _hdmaPending = false;
	bool _inDma = false;
	uint8_t _hdmaChannels = 0;
	uint8_t _nmiIrqDelayCounter = 0;
	uint8_t _requestedDmaChannels = 0;

	DmaChannelConfig _channel[8] = {};
	MemoryManager *_memoryManager;
	
	void CopyDmaByte(uint32_t addressBusA, uint16_t addressBusB, bool fromBtoA);

	void RunSingleTransfer(DmaChannelConfig &channel);
	void RunDma(DmaChannelConfig &channel);
	
	void RunHdmaTransfer(DmaChannelConfig &channel);
	void ProcessHdmaChannels(bool applyOverhead);

	void SyncStartDma();
	void SyncEndDma();

public:
	DmaController(MemoryManager *memoryManager);

	void Reset();

	bool HasNmiIrqDelay();
	
	void InitHdmaChannels();
	void BeginHdmaTransfer();

	void ProcessPendingTransfers();

	void Write(uint16_t addr, uint8_t value);
	uint8_t Read(uint16_t addr);

	void Serialize(Serializer &s) override;
};