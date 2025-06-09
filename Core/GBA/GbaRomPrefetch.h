#pragma once
#include "pch.h"
#include "GBA/GbaWaitStates.h"
#include "GBA/GbaConsole.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class GbaRomPrefetch final : ISerializable
{
private:
	GbaRomPrefetchState _state = {};
	GbaWaitStates* _waitStates = nullptr;
	GbaConsole* _console = nullptr;

	__forceinline bool IsEmpty()
	{
		return _state.ReadAddr == _state.PrefetchAddr;
	}
	
	__forceinline bool IsFull()
	{
		return _state.PrefetchAddr - _state.ReadAddr >= 16;
	}

	__forceinline bool IsRomBoundary()
	{
		return (_state.PrefetchAddr & 0x1FFFE) == 0;
	}

	__forceinline uint8_t WaitForPendingRead()
	{
		if(_state.ClockCounter == 0) {
			_state.ClockCounter = GetAccessClockCount();
		}

		uint8_t counter = _state.ClockCounter;
		ProcessRead();
		return counter;
	}

	__forceinline uint8_t ReadHalfWord()
	{
		uint8_t counter = GetAccessClockCount();
		ProcessRead();
		return counter;
	}

	__forceinline void ProcessRead()
	{
		_state.ReadAddr += 2;
		_state.PrefetchAddr += 2;
		_state.ClockCounter = 0;
		_state.Sequential = true;
	}

	__forceinline uint8_t GetAccessClockCount()
	{
		return _waitStates->GetPrefetchWaitStates(GbaAccessMode::HalfWord | (_state.Sequential ? GbaAccessMode::Sequential : 0), _state.PrefetchAddr);
	}

	__noinline void TriggerCpuFreeze()
	{
		//Prefetch unit can't read more data because it was filled and needs to be emptied to resume.
		//But the CPU needs more data to continue - this causes the system to hang (prefetcher_branch_thumb_arm_2 test)
		_console->SetCpuStopFlag();
	}

public:
	void Init(GbaWaitStates* waitStates, GbaConsole* console)
	{
		_waitStates = waitStates;
		_console = console;
	}

	GbaRomPrefetchState& GetState()
	{
		return _state;
	}

	bool Reset()
	{
		bool delay = _state.ClockCounter == 1;
		_state.Started = false;
		_state.Sequential = false;
		_state.WasFilled = false;
		_state.HitBoundary = false;
		_state.ReadAddr = 0;
		_state.PrefetchAddr = 0;
		_state.ClockCounter = 0;
		return delay;
	}

	void ForceNonSequential(uint32_t addr)
	{
		if(addr == _state.ReadAddr && _state.ClockCounter == 0 && !_state.Started) {
			//When the CPU jumps to the address the prefetch was about to read/fetch
			//and the prefetcher has been empty since its last reset (always ReadAddr==PrefetchAddr),
			//then the fetch counts as a non-sequential read
			_state.Sequential = false;
		}
	}

	__forceinline bool NeedExec(bool prefetchEnabled)
	{
		return _state.ReadAddr > 0 && !IsFull() && (prefetchEnabled || _state.ClockCounter > 0);
	}

	void Exec(uint8_t clocks, bool prefetchEnabled)
	{
		_state.Started = true;

		if(_state.WasFilled) {
			//Once the prefetch buffer is filled, it needs to be emptied completely
			//before any further prefetching is allowed
			return;
		}

		do {
			if(_state.ClockCounter == 0) {
				_state.ClockCounter = GetAccessClockCount();
				_state.Sequential = true;
			}

			if(--_state.ClockCounter == 0) {
				if(!_state.HitBoundary) {
					//Prefetch fails to increment past boundary and keeps trying to read the same address
					_state.PrefetchAddr += 2;
				}

				//Any cpu access after the prefetch stops should be non-sequential
				_console->ClearCpuSequentialFlag();

				if(IsRomBoundary()) {
					_state.HitBoundary = true;
				}

				if(IsFull()) {
					_state.WasFilled = true;
					break;
				}

				if(!prefetchEnabled) {
					break;
				}
			}
		} while(--clocks);
	}

	template<bool prefetchEnabled>
	__forceinline uint8_t Read(GbaAccessModeVal mode, uint32_t addr)
	{
		if(_state.WasFilled && IsEmpty()) {
			//Prefetch was paused because it was filled,
			//and it can resume because it's empty
			_state.WasFilled = false;
			_state.Sequential = false;
			_state.Started = false;
		}

		if constexpr(!prefetchEnabled) {
			if(IsEmpty()) {
				//Prefetcher is disabled & empty (but a prefetch read might be in-progress)
				if(_state.ClockCounter == 0) {
					//No prefetch is in progress - read normally
					return _waitStates->GetPrefetchWaitStates(mode, addr);
				} else {
					//Finish the current prefetch read
					uint8_t totalTime = WaitForPendingRead();
					_console->ClearCpuSequentialFlag();
					if(mode & GbaAccessMode::Word) {
						//If fetching a 32-bit value, add the time it'll take to read the next half-word (non sequential)
						totalTime += _waitStates->GetPrefetchWaitStates(GbaAccessMode::HalfWord, addr);
						
						//The previous read counts as the first non-sequential read, next one should be sequential
						_console->SetCpuSequentialFlag();
					}
					return totalTime;
				}
			}
		} else {
			if((addr & ((mode & GbaAccessMode::Word) ? ~0x03 : ~0)) != _state.ReadAddr) {
				//Restart prefetch, need to read an entire opcode
				uint8_t totalTime = Reset();
				_state.PrefetchAddr = addr;
				_state.ReadAddr = addr;
				totalTime += ReadHalfWord();
				if(mode & GbaAccessMode::Word) {
					totalTime += ReadHalfWord();
				}
				return totalTime;
			} else if(IsEmpty()) {
				//Prefetch in progress, wait until it ends
				uint8_t totalTime = WaitForPendingRead();
				if(mode & GbaAccessMode::Word) {
					//Need to fetch the next half-word, too
					totalTime += ReadHalfWord();
				}
				return totalTime;
			}
		}
		
		_state.ReadAddr += 2;
		if(mode & GbaAccessMode::Word) {
			//Need to read another half-word
			if(!IsEmpty()) {
				//Data is already available, return it without any additional delay
				_state.ReadAddr += 2;
				if(NeedExec(prefetchEnabled)) {
					Exec(1, prefetchEnabled);
				}
				return 1;
			} else {
				if(_state.WasFilled) {
					TriggerCpuFreeze();
				}

				//Prefetch in progress, wait until it ends
				if constexpr(!prefetchEnabled) {
					if(_state.ClockCounter == 0) {
						//Prefetch is disabled and no pending read is in progress, but the CPU needs to read an extra half-word (ARM mode)
						uint8_t totalTime = _waitStates->GetPrefetchWaitStates(GbaAccessMode::HalfWord, addr);
						//The previous read counts as the first non-sequential read, next one should be sequential
						_console->SetCpuSequentialFlag();
						return totalTime;
					}
				}
				return WaitForPendingRead();
			}
		} else {
			if(NeedExec(prefetchEnabled)) {
				Exec(1, prefetchEnabled);
			}
			return 1;
		}
	}
	
	void Serialize(Serializer& s) override
	{
		if(s.GetFormat() == SerializeFormat::Map) {
			return;
		}

		SV(_state.ClockCounter);
		SV(_state.ReadAddr);
		SV(_state.PrefetchAddr);
		SV(_state.WasFilled);
		SV(_state.Sequential);
		SV(_state.Started);
		SV(_state.HitBoundary);
	}
};