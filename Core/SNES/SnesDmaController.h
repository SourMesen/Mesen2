#pragma once
#include "pch.h"
#include "SNES/SnesCpuTypes.h"
#include "SNES/DmaControllerTypes.h"
#include "Utilities/ISerializable.h"

class SnesMemoryManager;

class SnesDmaController final : public ISerializable
{
private:
	static constexpr uint8_t HdmaChannelFlag = 0x40;

	SnesDmaControllerState _state = {};

	bool _needToProcess = false;
	bool _hdmaPending = false;
	bool _hdmaInitPending = false;
	bool _dmaStartDelay = false;
	bool _dmaPending = false;
	uint32_t _dmaClockCounter = 0;
	
	uint8_t _activeChannel = 0; //Used by debugger's event viewer

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

	SnesDmaControllerState& GetState();

	void Reset();

	void BeginHdmaTransfer();
	void BeginHdmaInit();

	__forceinline bool HasPendingTransfer() { return _needToProcess; }

	bool ProcessPendingTransfers();

	void Write(uint16_t addr, uint8_t value);
	uint8_t Read(uint16_t addr);

	uint8_t GetActiveChannel();
	DmaChannelConfig GetChannelConfig(uint8_t channel);

	void Serialize(Serializer &s) override;
};