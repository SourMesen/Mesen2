#pragma once
#include "pch.h"
#include "WS/APU/WsApu.h"
#include "WS/WsTypes.h"

class WsApuCh1
{
private:
	WsApu* _apu = nullptr;
	WsApuCh1State* _state = nullptr;

public:
	WsApuCh1(WsApu* apu, WsApuCh1State& state)
	{
		_state = &state;
		_apu = apu;
	}

	void Exec()
	{
		if(!_state->Enabled) {
			return;
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

		uint8_t sample = _apu->ReadSample(0, _state->SamplePosition);

		_state->LeftOutput = _state->LeftVolume * sample;
		_state->RightOutput = _state->RightVolume * sample;
	}
};
