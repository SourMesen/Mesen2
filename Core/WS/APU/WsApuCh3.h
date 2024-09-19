#pragma once
#include "pch.h"
#include "WS/APU/WsApu.h"
#include "WS/WsTypes.h"

class WsApuCh3
{
private:
	WsApu* _apu = nullptr;
	WsApuCh3State* _state = nullptr;

public:
	WsApuCh3(WsApu* apu, WsApuCh3State& state)
	{
		_state = &state;
		_apu = apu;
	}

	void Exec()
	{
		if(!_state->Enabled) {
			return;
		}

		if(_state->SweepEnabled) {
			_state->SweepScaler++;
			if(_state->UseSweepCpuClock || _state->SweepScaler >= 0x2000) {
				_state->SweepScaler = 0;
				
				if(_state->SweepTimer == 0) {
					_state->SweepTimer = _state->SweepPeriod;
					_state->Frequency = (_state->Frequency + _state->SweepValue) & 0xFFF;
				} else {
					_state->SweepTimer--;
				}
			}
		}

		if(_state->Timer == 0) {
			_state->Timer = 2047 - _state->Frequency;
			_state->SamplePosition = (_state->SamplePosition + 1) & 0x1F;
		} else {
			_state->Timer--;
		}
	}

	void UpdateOutput()
	{
		if(!_state->Enabled) {
			return;
		}

		uint8_t sample = _apu->ReadSample(2, _state->SamplePosition);

		_state->LeftOutput = _state->LeftVolume * sample;
		_state->RightOutput = _state->RightVolume * sample;
	}
};
