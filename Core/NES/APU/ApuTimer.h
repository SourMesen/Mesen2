#pragma once
#include "pch.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"
#include "NES/INesMemoryHandler.h"
#include "NES/NesConsole.h"
#include "NES/NesSoundMixer.h"

class ApuTimer : public ISerializable
{
private:
	uint32_t _previousCycle;
	uint16_t _timer = 0;
	uint16_t _period = 0;
	int8_t _lastOutput = 0;

	AudioChannel _channel = AudioChannel::Square1;
	NesSoundMixer* _mixer = nullptr;

public:
	ApuTimer(AudioChannel channel, NesSoundMixer* mixer)
	{
		_channel = channel;
		_mixer = mixer;
		Reset(false);
	}	

	void Reset(bool softReset)
	{
		_timer = 0;
		_period = 0;
		_previousCycle = 0;
		_lastOutput = 0;
	}

	void Serialize(Serializer& s) override
	{
		if(!s.IsSaving()) {
			_previousCycle = 0;
		}

		SV(_timer); SV(_period); SV(_lastOutput);
	}

	__forceinline void AddOutput(int8_t output)
	{
		if(output != _lastOutput) {
			_mixer->AddDelta(_channel, _previousCycle, output - _lastOutput);
			_lastOutput = output;
		}
	}

	int8_t GetLastOutput()
	{
		return _lastOutput;
	}

	__forceinline bool Run(uint32_t targetCycle)
	{
		int32_t cyclesToRun = targetCycle - _previousCycle;
		
		if(cyclesToRun > _timer) {
			_previousCycle += _timer + 1;
			_timer = _period;
			return true;
		}

		_timer -= cyclesToRun;
		_previousCycle = targetCycle;
		return false;
	}

	void SetPeriod(uint16_t period)
	{
		_period = period;
	}

	uint16_t GetPeriod()
	{
		return _period;
	}

	uint16_t GetTimer()
	{
		return _timer;
	}

	void SetTimer(uint16_t timer)
	{
		_timer = timer;
	}

	__forceinline void EndFrame()
	{
		_previousCycle = 0;
	}
};