#pragma once
#include "pch.h"
#include "WS/APU/WsApu.h"
#include "WS/WsTypes.h"

class WsApuCh4
{
private:
	WsApu* _apu = nullptr;
	WsApuCh4State* _state = nullptr;

public:
	WsApuCh4(WsApu* apu, WsApuCh4State& state)
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
			_state->SamplePosition = (_state->SamplePosition + 1) & 0x1F; //todows does this happen even when lfsr is enabled?

			if(_state->LfsrEnabled) {
				uint8_t newBit = ((_state->Lfsr >> 7) ^ (_state->Lfsr >> _state->TapShift) ^ 0x01) & 0x01;
				_state->Lfsr = ((_state->Lfsr << 1) | newBit) & 0x7FFF;
			}
		} else {
			_state->Timer--;
		}
	}

	void UpdateOutput()
	{
		if(!_state->Enabled) {
			return;
		}

		uint8_t sample;
		if(_state->NoiseEnabled) {
			sample = (_state->Lfsr & 0x01) ? 0x0F : 0;
		} else {
			sample = _apu->ReadSample(3, _state->SamplePosition);
		}

		_state->LeftOutput = _state->LeftVolume * sample;
		_state->RightOutput = _state->RightVolume * sample;
	}
};
