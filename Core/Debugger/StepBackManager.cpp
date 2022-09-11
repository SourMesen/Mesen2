#include "pch.h"
#include "Debugger/StepBackManager.h"
#include "Debugger/IDebugger.h"
#include "Shared/Emulator.h"
#include "Shared/SaveStateManager.h"
#include "Shared/NotificationManager.h"
#include "Shared/RewindManager.h"

StepBackManager::StepBackManager(Emulator* emu, IDebugger* debugger)
{
	_emu = emu;
	_debugger = debugger;
}

void StepBackManager::StepBack()
{
	if(!_active) {
		_targetClock = _debugger->GetCpuCycleCount();
		_active = true;
		_allowRetry = true;
		_stateClockLimit = StepBackManager::DefaultClockLimit;
	}
}

bool StepBackManager::CheckStepBack()
{
	if(!_active) {
		return false;
	}

	uint64_t clock = _debugger->GetCpuCycleCount();

	if(!_emu->GetRewindManager()->IsStepBack()) {
		if(_cache.size() > 1) {
			//Check to see if previous instruction is already in cache
			if(_cache.back().Clock == _targetClock) {
				//End of cache is the current instruction, remove it first
				_cache.pop_back();
				if(_cache.size()) {
					//If cache isn't empty, load the last state
					_emu->GetRewindManager()->SetIgnoreLoadState(true);
					_emu->Deserialize(_cache.back().SaveState, SaveStateManager::FileFormatVersion, true);
					_emu->GetRewindManager()->SetIgnoreLoadState(false);

					_emu->GetRewindManager()->StopRewinding(true);
					_active = false;
					_prevClock = clock;
					return true;
				}
			} else {
				//On mismatch, clear cache and rewind normally instead
				_cache.clear();
			}
		}

		//Start rewinding on next instruction after StepBack() is called
		_cache.clear();
		_emu->GetRewindManager()->StartRewinding(true);
		clock = _debugger->GetCpuCycleCount();
	}

	if(clock < _targetClock && _targetClock - clock < _stateClockLimit) {
		//Create a save state every instruction for the last X clocks
		_cache.push_back({});
		_cache.back().Clock = clock;
		_emu->Serialize(_cache.back().SaveState, true, 0);
	}

	if(clock >= _targetClock) {
		//If the CPU is back to where it was before step back, check if the cache contains data
		if(_cache.size() > 0) {
			//If it does, load the last state
			_emu->GetRewindManager()->SetIgnoreLoadState(true);
			_emu->Deserialize(_cache.back().SaveState, SaveStateManager::FileFormatVersion, true);
			_emu->GetRewindManager()->SetIgnoreLoadState(false);
		} else if(_allowRetry && clock > _prevClock && (clock - _prevClock) > StepBackManager::DefaultClockLimit) {
			//Cache is empty, this can happen when a single instruction takes more than X clocks (e.g block transfers, dma)
			//In this case, re-run the step back process again but start recordings state earlier
			_emu->GetRewindManager()->StopRewinding(true);
			_emu->GetRewindManager()->StartRewinding(true);
			_stateClockLimit = (clock - _prevClock) + StepBackManager::DefaultClockLimit;
			_allowRetry = false;
			return false;
		}

		//Stop rewinding, even if the target was not found
		_emu->GetRewindManager()->StopRewinding(true);
		_active = false;
		_prevClock = clock;
		return true;
	}

	_prevClock = clock;
	return false;
}
