#include "stdafx.h"
#include "PCE/PceTimer.h"
#include "PCE/PceMemoryManager.h"

PceTimer::PceTimer(PceMemoryManager* memoryManager)
{
	_memoryManager = memoryManager;

	//Counter is 0 on power on
	_counter = 0;
}

void PceTimer::Exec(uint8_t clocksToRun)
{
	if(!_enabled) {
		return;
	}

	do {
		_scaler -= 3;
		if(_scaler == 0) {
			_scaler = 1024 * 3;
			if(_counter == 0) {
				_counter = _reloadValue;
				_memoryManager->SetIrqSource(PceIrqSource::TimerIrq);
			} else {
				_counter--;
			}
		}
		clocksToRun -= 3;
	} while(clocksToRun > 0);
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
	if(_counter == 0 && _scaler <= 5 * 3) {
		//When the timer is about to expire and rolls back to $7F,
		//there is a slight delay where the register returns $7F instead
		//of the reload value. Some games depend on this (e.g Battle Royale)
		//TODO what's the correct timing for this?
		return 0x7F;
	} else {
		return _counter;
	}
}
