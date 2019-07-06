#pragma once
#include "stdafx.h"
#include "CpuTypes.h"
#include "../Utilities/ISerializable.h"

class MemoryManager;

struct DmaChannelConfig
{
	bool DmaActive;

	bool InvertDirection;
	bool Decrement;
	bool FixedTransfer;
	bool HdmaIndirectAddressing;
	uint8_t TransferMode;

	uint16_t SrcAddress;
	uint8_t SrcBank;

	uint16_t TransferSize;
	uint8_t DestAddress;

	uint16_t HdmaTableAddress;
	uint8_t HdmaBank;
	uint8_t HdmaLineCounterAndRepeat;
	bool DoTransfer;
	bool HdmaFinished;

	bool UnusedFlag;
};

class DmaController final : public ISerializable
{
private:
	bool _needToProcess = false;
	bool _hdmaPending = false;
	bool _hdmaInitPending = false;
	bool _dmaStartDelay = false;
	uint8_t _hdmaChannels = 0;
	bool _dmaPending = false;
	uint64_t _dmaStartClock = 0;
	
	uint8_t _activeChannel = 0; //Used by debugger's event viewer

	DmaChannelConfig _channel[8] = {};
	MemoryManager *_memoryManager;
	
	void CopyDmaByte(uint32_t addressBusA, uint16_t addressBusB, bool fromBtoA);

	void RunDma(DmaChannelConfig &channel);
	
	void RunHdmaTransfer(DmaChannelConfig &channel);
	bool ProcessHdmaChannels();
	bool InitHdmaChannels();

	void SyncStartDma();
	void SyncEndDma();
	void UpdateNeedToProcessFlag();

	bool HasActiveDmaChannel();

public:
	DmaController(MemoryManager *memoryManager);

	void Reset();

	void BeginHdmaTransfer();
	void BeginHdmaInit();

	bool ProcessPendingTransfers();

	void Write(uint16_t addr, uint8_t value);
	uint8_t Read(uint16_t addr);

	uint8_t GetActiveChannel();
	DmaChannelConfig GetChannelConfig(uint8_t channel);

	void Serialize(Serializer &s) override;
};