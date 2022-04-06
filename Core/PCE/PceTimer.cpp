#include "stdafx.h"
#include "PCE/PceTimer.h"
#include "PCE/PceMemoryManager.h"

PceTimer::PceTimer(PceMemoryManager* memoryManager)
{
	_memoryManager = memoryManager;
}

void PceTimer::Exec(int8_t clocksToRun)
{
	if(!_enabled) {
		return;
	}

	while(clocksToRun) {
		_scaler--;
		if(_scaler == 0) {
			_scaler = 1024 * 3;
			if(_counter == 0) {
				_counter = _reloadValue;
				_memoryManager->SetIrqSource(PceIrqSource::TimerIrq);
			} else {
				_counter--;
			}
		}
		clocksToRun--;
	}
}

void PceTimer::Write(uint16_t addr, uint8_t value)
{
	if(addr & 0x01) {
		bool enabled = (value & 0x01) != 0;
		if(_enabled != enabled) {
			_enabled = enabled;
			_scaler = 1024 * 3;
			_counter = _reloadValue;
		}
	} else {
		_reloadValue = value & 0x7F;
	}
}

uint8_t PceTimer::Read(uint16_t addr)
{
	return _counter;
}
