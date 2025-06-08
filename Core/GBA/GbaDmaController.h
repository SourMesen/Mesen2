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
	bool _dmaRunning = false;
	bool _needStart = false;
	uint32_t _idleCycleCounter = 0;

	int GetPendingDmaIndex();
	void RunDma(GbaDmaChannel& ch, uint8_t chIndex);

public:
	void Init(GbaCpu* cpu, GbaMemoryManager* memoryManager, GbaRomPrefetch* prefetcher);

	GbaDmaControllerState& GetState();

	bool IsVideoCaptureDmaEnabled();
	bool IsRunning() { return _dmaRunning; }

	int8_t DebugGetActiveChannel();

	void TriggerDmaChannel(GbaDmaTrigger trigger, uint8_t channel, bool forceStop = false);
	void TriggerDma(GbaDmaTrigger trigger);

	__forceinline bool HasPendingDma() { return _needStart; }
	__noinline void RunPendingDma(bool allowStartDma);

	__forceinline void ResetIdleCounter(GbaAccessModeVal& mode)
	{
		if(_idleCycleCounter) {
			//Next access immediately after DMA should always be non-sequential
			mode &= ~GbaAccessMode::Sequential;
		}
		_idleCycleCounter = 0;
	}

	__forceinline void ResetIdleCounter()
	{
		_idleCycleCounter = 0;
	}

	bool CanRunInParallelWithDma();

	uint8_t ReadRegister(uint32_t addr);
	void WriteRegister(uint32_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};