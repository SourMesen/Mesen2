#pragma once
#include "pch.h"
#include "NES/NesConsole.h"
#include "NES/APU/NesApu.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class ApuLengthCounter : public ISerializable
{
private:
	static constexpr uint8_t _lcLookupTable[32] = { 10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14, 12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30 };
	NesConsole* _console = nullptr;
	AudioChannel _channel = AudioChannel::Square1;
	bool _newHaltValue = false;

protected:
	bool _enabled = false;
	bool _halt = false;
	uint8_t _counter = 0;
	uint8_t _reloadValue = 0;
	uint8_t _previousValue = 0;

public:
	void InitializeLengthCounter(bool haltFlag)
	{
		_console->GetApu()->SetNeedToRun();
		_newHaltValue = haltFlag;
	}

	void LoadLengthCounter(uint8_t value)
	{
		if(_enabled) {
			_reloadValue = _lcLookupTable[value];
			_previousValue = _counter;
			_console->GetApu()->SetNeedToRun();
		}
	}
	
	ApuLengthCounter(AudioChannel channel, NesConsole* console)
	{
		_channel = channel;
		_console = console;
	}
	
	void Reset(bool softReset)
	{
		if(softReset) {
			_enabled = false;
			if(_channel != AudioChannel::Triangle) {
				//"At reset, length counters should be enabled, triangle unaffected"
				_halt = false;
				_counter = 0;
				_newHaltValue = false;
				_reloadValue = 0;
				_previousValue = 0;
			}
		} else {
			_enabled = false;
			_halt = false;
			_counter = 0;
			_newHaltValue = false;
			_reloadValue = 0;
			_previousValue = 0;
		}
	}

	void Serialize(Serializer &s) override
	{
		SV(_enabled); SV(_halt); SV(_newHaltValue); SV(_counter); SV(_previousValue); SV(_reloadValue);
	}

	bool GetStatus()
	{
		return _counter > 0;
	}

	bool IsHalted()
	{
		return _halt;
	}
	
	void ReloadCounter()
	{
		if(_reloadValue) {
			if(_counter == _previousValue) {
				_counter = _reloadValue;
			}
			_reloadValue = 0;
		}

		_halt = _newHaltValue;
	}

	void TickLengthCounter()
	{
		if(_counter > 0 && !_halt) {
			_counter--;
		}
	}

	void SetEnabled(bool enabled)
	{
		if(!enabled) {
			_counter = 0;
		}
		_enabled = enabled;
	}

	bool IsEnabled()
	{
		return _enabled;
	}

	ApuLengthCounterState GetState()
	{
		ApuLengthCounterState state;
		state.Counter = _counter;
		state.Halt = _halt;
		state.ReloadValue = _reloadValue;
		return state;
	}
};
