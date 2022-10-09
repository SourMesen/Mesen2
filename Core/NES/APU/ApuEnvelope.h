#pragma once
#include "pch.h"
#include "NES/APU/ApuLengthCounter.h"
#include "NES/NesConsole.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class ApuEnvelope : public ISerializable
{
private:
	bool _constantVolume = false;
	uint8_t _volume = 0;

	bool _start = false;
	int8_t _divider = 0;
	uint8_t _counter = 0;

public:
	ApuLengthCounter LengthCounter;

	ApuEnvelope(AudioChannel channel, NesConsole* console) : LengthCounter(channel, console)
	{
	}

	void InitializeEnvelope(uint8_t regValue)
	{
		LengthCounter.InitializeLengthCounter((regValue & 0x20) == 0x20);
		_constantVolume = (regValue & 0x10) == 0x10;
		_volume = regValue & 0x0F;
	}

	void ResetEnvelope()
	{
		_start = true;
	}
	
	uint32_t GetVolume()
	{
		if(LengthCounter.GetStatus()) {
			if(_constantVolume) {
				return _volume;
			} else {
				return _counter;
			}
		} else {
			return 0;
		}
	}

	void Reset(bool softReset)
	{
		LengthCounter.Reset(softReset);
		_constantVolume = false;
		_volume = 0;
		_start = false;
		_divider = 0;
		_counter = 0;
	}

	void Serialize(Serializer& s) override
	{
		SV(_constantVolume); SV(_volume); SV(_start); SV(_divider); SV(_counter);
		SV(LengthCounter);
	}

	void TickEnvelope()
	{
		if(!_start) {
			_divider--;
			if(_divider < 0) {
				_divider = _volume;
				if(_counter > 0) {
					_counter--;
				} else if(LengthCounter.IsHalted()) {
					_counter = 15;
				}
			}
		} else {
			_start = false;
			_counter = 15;
			_divider = _volume;
		}
	}

	ApuEnvelopeState GetState()
	{
		ApuEnvelopeState state;
		state.ConstantVolume = _constantVolume;
		state.Counter = _counter;
		state.Divider = _divider;
		state.Loop = LengthCounter.IsHalted();
		state.StartFlag = _start;
		state.Volume = _volume;
		return state;
	}
};
