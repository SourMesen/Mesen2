#pragma once
#include "pch.h"
#include "GBA/GbaMemoryManager.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class GbaRomPrefetch final : ISerializable
{
private:
	GbaRomPrefetchState _state = {};
	GbaMemoryManager* _memoryManager = nullptr;

	__forceinline bool IsEmpty()
	{
		return _state.ReadAddr == _state.PrefetchAddr;
	}
	
	__forceinline bool IsFull()
	{
		return _state.PrefetchAddr - _state.ReadAddr >= 16;
	}

	__forceinline uint8_t WaitForPendingRead()
	{
		uint8_t counter = _state.ClockCounter;
		Exec(counter);
		_state.ReadAddr += 2;
		return counter;
	}

	__forceinline uint8_t ReadHalfWord(GbaAccessModeVal mode)
	{
		uint8_t counter = _memoryManager->GetWaitStates(GbaAccessMode::HalfWord | mode, _state.PrefetchAddr);
		_state.ClockCounter = counter;
		Exec(counter);
		_state.ReadAddr += 2;
		return counter;
	}

public:
	void Init(GbaMemoryManager* memoryManager)
	{
		_memoryManager = memoryManager;
	}

	GbaRomPrefetchState& GetState()
	{
		return _state;
	}

	bool Reset()
	{
		bool delay = _state.ClockCounter == 1;
		_state.ReadAddr = 0;
		_state.PrefetchAddr = 0;
		_state.ClockCounter = 0;
		return delay;
	}

	__forceinline void SetSuspendState(bool suspended)
	{
		//Prefetch is suspended by DMA once DMA accesses ROM
		_state.Suspended = suspended;
	}

	void Exec(uint8_t clocks)
	{
		if(_state.Suspended || IsFull()) {
			return;
		}

		do {
			if(--_state.ClockCounter == 0) {
				_state.PrefetchAddr += 2;
				_state.ClockCounter = _memoryManager->GetWaitStates(GbaAccessMode::HalfWord | GbaAccessMode::Sequential, _state.PrefetchAddr);
			}
		} while(--clocks);
	}

	__forceinline uint8_t Read(GbaAccessModeVal mode, uint32_t addr)
	{
		if(addr != _state.ReadAddr) {
			//Restart prefetch, need to read an entire opcode
			_state.PrefetchAddr = addr;
			_state.ReadAddr = addr;
			uint8_t totalTime = ReadHalfWord(GbaAccessMode::HalfWord);
			if(mode & GbaAccessMode::Word) {
				totalTime += ReadHalfWord(GbaAccessMode::HalfWord | GbaAccessMode::Sequential);
			}
			return totalTime;
		} else if(IsEmpty()) {
			//Prefetch in progress, wait until it ends
			uint8_t totalTime = WaitForPendingRead();
			if(mode & GbaAccessMode::Word) {
				//Need to fetch the next half-word, too
				totalTime += ReadHalfWord(GbaAccessMode::HalfWord | GbaAccessMode::Sequential);
			}
			return totalTime;
		}
		
		_state.ReadAddr += 2;
		if(mode & GbaAccessMode::Word) {
			//Need to read another half-word
			if(!IsEmpty()) {
				//Data is already available, return it without any additional delay
				_state.ReadAddr += 2;
				Exec(1);
				return 1;
			} else {
				//Prefetch in progress, wait until it ends
				return WaitForPendingRead();
			}
		} else {
			Exec(1);
			return 1;
		}
	}
	
	void Serialize(Serializer& s) override
	{
		SV(_state.ClockCounter);
		SV(_state.ReadAddr);
		SV(_state.PrefetchAddr);
		SV(_state.Suspended);
	}
};