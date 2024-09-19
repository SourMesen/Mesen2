#pragma once
#include "pch.h"
#include "WS/APU/WsApu.h"
#include "WS/WsTypes.h"

class WsApuCh2
{
private:
	WsApu* _apu = nullptr;
	WsApuCh2State* _state = nullptr;

public:
	WsApuCh2(WsApu* apu, WsApuCh2State& state)
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

		if(_state->PcmEnabled) {
			uint8_t sample = _state->GetVolume();
			_state->LeftOutput = sample >> (_state->MaxPcmVolumeLeft ? 0 : (_state->HalfPcmVolumeLeft ? 1 : 8));
			_state->RightOutput = sample >> (_state->MaxPcmVolumeRight ? 0 : (_state->HalfPcmVolumeRight ? 1 : 8));
		} else {
			uint8_t sample = _apu->ReadSample(1, _state->SamplePosition);
			_state->LeftOutput = _state->LeftVolume * sample;
			_state->RightOutput = _state->RightVolume * sample;
		}
	}
};
