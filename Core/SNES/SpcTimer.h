#pragma once
#include "pch.h"
#include "Utilities/Serializer.h"

template<uint8_t rate>
class SpcTimer
{
private:
	bool _enabled = false;
	bool _timersEnabled = true;
	uint8_t _output = 0x0F; //"On power on, all three TnOUT have the value $F. On reset, they are $0."
	uint8_t _stage0 = 0;
	uint8_t _stage1 = 0;
	uint8_t _prevStage1 = 0;
	uint8_t _stage2 = 0;
	uint8_t _target = 0;

	void ClockTimer()
	{
		uint8_t currentState = _stage1;
		if(!_timersEnabled) {
			//All timers are disabled
			currentState = 0;
		}

		uint8_t prevState = _prevStage1;
		_prevStage1 = currentState;
		if(!_enabled || !prevState || currentState) {
			//Only clock on 1->0 transitions, when the timer is enabled
			return;
		}

		if(++_stage2 == _target) {
			_stage2 = 0;
			_output++;
		}
	}

public:
	void Reset()
	{
		_output = 0;
	}

	void SetEnabled(bool enabled)
	{
		if(!_enabled && enabled) {
			_stage2 = 0;
			_output = 0;
		}
		_enabled = enabled;
	}

	void SetGlobalEnabled(bool enabled)
	{
		_timersEnabled = enabled;
		ClockTimer();
	}

	void Run(uint8_t step)
	{
		_stage0 += step;
		if(_stage0 >= rate) {
			_stage1 ^= 0x01;
			_stage0 -= rate;

			ClockTimer();
		}
	}

	void SetTarget(uint8_t target)
	{
		_target = target;
	}

	uint8_t DebugRead()
	{
		return _output & 0x0F;
	}

	uint8_t GetOutput()
	{
		uint8_t value = _output & 0x0F;
		_output = 0;
		return value;
	}

	void SetOutput(uint8_t value)
	{
		//Used when loading SPC files
		_output = value;
	}

	void Serialize(Serializer &s)
	{
		SV(_stage0); SV(_stage1); SV(_stage2); SV(_output); SV(_target); SV(_enabled); SV(_timersEnabled); SV(_prevStage1);
	}
};