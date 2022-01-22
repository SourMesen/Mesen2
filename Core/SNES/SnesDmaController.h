#pragma once
#include "stdafx.h"
#include "SNES/SnesCpuTypes.h"
#include "SNES/DmaControllerTypes.h"
#include "Utilities/ISerializable.h"

class SnesMemoryManager;

class SnesDmaController final : public ISerializable
{
private:
	static constexpr uint8_t HdmaChannelFlag = 0x40;

	bool _needToProcess = false;
	bool _hdmaPending = false;
	bool _hdmaInitPending = false;
	bool _dmaStartDelay = false;
	uint8_t _hdmaChannels = 0;
	bool _dmaPending = false;
	uint64_t _dmaStartClock = 0;
	
	uint8_t _activeChannel = 0; //Used by debugger's event viewer

	DmaChannelConfig _channel[8] = {};
	SnesMemoryManager *_memoryManager;
	
	void CopyDmaByte(uint32_t addressBusA, uint16_t addressBusB, bool fromBtoA);

	void RunDma(DmaChannelConfig &channel);
	
	void RunHdmaTransfer(DmaChannelConfig &channel);
	bool ProcessHdmaChannels();
	bool IsLastActiveHdmaChannel(uint8_t channel);
	bool InitHdmaChannels();

	void SyncStartDma();
	void SyncEndDma();
	void UpdateNeedToProcessFlag();

	bool HasActiveDmaChannel();

public:
	SnesDmaController(SnesMemoryManager *memoryManager);

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