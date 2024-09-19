#pragma once
#include "pch.h"
#include "WS/WsTypes.h"
#include "Utilities/BitUtilities.h"

class WsHyperVoice
{
private:
	WsApuHyperVoiceState* _state = nullptr;

public:
	WsHyperVoice(WsApuHyperVoiceState& state)
	{
		_state = &state;
	}

	int16_t ConvertSampleToOutput(uint8_t sample)
	{
		int32_t output = 0;
		switch(_state->ScalingMode) {
			case WsHyperVoiceScalingMode::Unsigned: output = sample << 8; break;
			case WsHyperVoiceScalingMode::UnsignedNegated: output = 0xFFFF0000 | (sample << 8); break;
			case WsHyperVoiceScalingMode::Signed: output = ((int32_t)(int8_t)sample) << 8; break;
			case WsHyperVoiceScalingMode::None: output = (int16_t)(sample << 8);
		}

		if(_state->ScalingMode != WsHyperVoiceScalingMode::None) {
			output >>= _state->Shift;
		}
		return output;
	}

	void Exec()
	{
		if(!_state->Enabled) {
			return;
		}

		if(_state->Timer == 0) {
			_state->Timer = _state->Divisor;

			_state->LeftOutput = ConvertSampleToOutput(_state->LeftSample);
			_state->RightOutput = ConvertSampleToOutput(_state->RightSample);
		} else {
			_state->Timer--;
		}
	}

	uint8_t Read(uint16_t port)
	{
		switch(port) {
			case 0x6A: return _state->ControlLow;
			case 0x6B: return _state->ControlHigh;
		}

		//TODOWS open bus
		return 0x90;
	}

	void WriteStereoInput(uint8_t sample)
	{
		if(_state->UpdateRightValue) {
			_state->RightSample = sample;
		} else {
			_state->LeftSample = sample;
		}
		_state->UpdateRightValue = !_state->UpdateRightValue;
	}

	void WriteDma(uint8_t sample)
	{
		switch(_state->ChannelMode) {
			case WsHyperVoiceChannelMode::Stereo:
				WriteStereoInput(sample);
				break;

			case WsHyperVoiceChannelMode::MonoLeft:
				_state->LeftSample = sample;
				break;

			case WsHyperVoiceChannelMode::MonoRight:
				_state->RightSample = sample;
				break;

			case WsHyperVoiceChannelMode::MonoBoth:
				_state->LeftSample = sample;
				_state->RightSample = sample;
				break;
		}
	}

	void Write(uint16_t port, uint8_t value)
	{
		switch(port) {
			case 0x64: BitUtilities::SetBits<0>(_state->LeftOutput, value); break;
			case 0x65: BitUtilities::SetBits<8>(_state->LeftOutput, value); break;
			case 0x66: BitUtilities::SetBits<0>(_state->RightOutput, value); break;
			case 0x67: BitUtilities::SetBits<8>(_state->RightOutput, value); break;
		
			case 0x69: WriteStereoInput(value); break;

			case 0x6A: {
				_state->Shift = value & 0x03;
				_state->ScalingMode = (WsHyperVoiceScalingMode)((value >> 2) & 0x03);
				_state->Enabled = value & 0x80;
				uint8_t dividor = (value >> 4) & 0x07;
				switch(dividor) {
					default: _state->Divisor = dividor; break;
					case 6: _state->Divisor = 7; break;
					case 7: _state->Divisor = 11; break;
				}
				_state->ControlLow = value;
				break;
			}

			case 0x6B: {
				bool reset = (value & 0x10);
				if(reset) {
					_state->UpdateRightValue = false;
				}
				_state->ChannelMode = (WsHyperVoiceChannelMode)((value >> 5) & 0x03);
				_state->ControlHigh = value & 0x6F;
				break;
			}
		}
	}
};