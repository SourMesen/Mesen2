#pragma once
#include "pch.h"
#include "GBA/GbaTypes.h"
#include "Utilities/ISerializable.h"

class GbaMemoryManager;
class GbaApu;

class GbaTimer final : public ISerializable
{
private:
	GbaTimersState _state = {};
	GbaMemoryManager* _memoryManager = nullptr;
	GbaApu* _apu = nullptr;
	bool _hasPendingTimers = false;
	bool _hasPendingWrites = false;

	void ClockTimer(int i);

	template<int i> __forceinline void ProcessTimer(uint64_t masterClock)
	{
		if(_state.Timer[i].ProcessTimer && (masterClock & _state.Timer[i].PrescaleMask) == 0) {
			if(++_state.Timer[i].Timer == 0) {
				ClockTimer(i);
			}
		}
	}
	
	void TriggerUpdate(GbaTimerState& timer);

public:
	void Init(GbaMemoryManager* memoryManager, GbaApu* apu);

	GbaTimersState& GetState() { return _state; }

	__forceinline void Exec(uint64_t masterClock)
	{
		ProcessTimer<0>(masterClock);
		ProcessTimer<1>(masterClock);
		ProcessTimer<2>(masterClock);
		ProcessTimer<3>(masterClock);
	}

	void WriteRegister(uint32_t addr, uint8_t value);
	uint8_t ReadRegister(uint32_t addr);
	
	bool HasPendingTimers() { return _hasPendingTimers; }
	bool HasPendingWrites() { return _hasPendingWrites; }
	
	void ProcessPendingTimers();
	void ProcessPendingWrites();

	void Serialize(Serializer& s) override;
};