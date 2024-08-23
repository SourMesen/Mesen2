#pragma once
#include "pch.h"
#include "GBA/GbaTypes.h"
#include "Utilities/ISerializable.h"

class GbaMemoryManager;
class GbaCpu;
class GbaRomPrefetch;

class GbaDmaController final : public ISerializable
{
private:
	GbaCpu* _cpu = nullptr;
	GbaMemoryManager* _memoryManager = nullptr;
	GbaRomPrefetch* _prefetcher = nullptr;
	GbaDmaControllerState _state = {};
	int8_t _dmaActiveChannel = -1;
	bool _dmaPending = false;
	uint8_t _dmaStartDelay = 0;

	void RunDma(GbaDmaChannel& ch, uint8_t chIndex);

public:
	void Init(GbaCpu* cpu, GbaMemoryManager* memoryManager, GbaRomPrefetch* prefetcher);

	GbaDmaControllerState& GetState();

	bool IsVideoCaptureDmaEnabled();

	int8_t DebugGetActiveChannel();

	void TriggerDmaChannel(GbaDmaTrigger trigger, uint8_t channel, bool forceStop = false);
	void TriggerDma(GbaDmaTrigger trigger);

	__forceinline bool HasPendingDma() { return _dmaStartDelay > 0; }
	__noinline void RunPendingDma(bool allowStartDma);

	uint8_t ReadRegister(uint32_t addr);
	void WriteRegister(uint32_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};