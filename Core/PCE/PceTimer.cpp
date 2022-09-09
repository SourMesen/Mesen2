#include "pch.h"
#include "PCE/PceTimer.h"
#include "PCE/PceConsole.h"
#include "PCE/PceMemoryManager.h"

PceTimer::PceTimer(PceConsole* console)
{
	_console = console;

	//Counter is 0 on power on
	_state.Counter = 0;
	_state.Scaler = 1024 * 3;
}

void PceTimer::Exec()
{
	if(!_state.Enabled) {
		return;
	}

	_state.Scaler -= 3;
	if(_state.Scaler == 0) {
		_state.Scaler = 1024 * 3;
		if(_state.Counter == 0) {
			_state.Counter = _state.ReloadValue;
			_console->GetMemoryManager()->SetIrqSource(PceIrqSource::TimerIrq);
		} else {
			_state.Counter--;
		}
	}
}

void PceTimer::Write(uint16_t addr, uint8_t value)
{
	if(addr & 0x01) {
		bool enabled = (value & 0x01) != 0;
		if(_state.Enabled != enabled) {
			_state.Enabled = enabled;
			_state.Scaler = 1024 * 3;
			_state.Counter = _state.ReloadValue;
		}
	} else {
		_state.ReloadValue = value & 0x7F;
	}
}

uint8_t PceTimer::Read(uint16_t addr)
{
	if(_state.Counter == 0 && _state.Scaler <= 5 * 3) {
		//When the timer is about to expire and rolls back to $7F,
		//there is a slight delay where the register returns $7F instead
		//of the reload value. Some games depend on this (e.g Battle Royale)
		//TODO what's the correct timing for this?
		return 0x7F;
	} else {
		return _state.Counter;
	}
}

void PceTimer::Serialize(Serializer& s)
{
	SV(_state.ReloadValue);
	SV(_state.Counter);
	SV(_state.Scaler);
	SV(_state.Enabled);
}
