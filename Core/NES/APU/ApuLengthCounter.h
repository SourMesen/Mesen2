#pragma once
#include "stdafx.h"
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
	bool _lengthCounterHalt = false;
	uint8_t _lengthCounter = 0;
	uint8_t _lengthCounterReloadValue = 0;
	uint8_t _lengthCounterPreviousValue = 0;

public:
	void InitializeLengthCounter(bool haltFlag)
	{
		_console->GetApu()->SetNeedToRun();
		_newHaltValue = haltFlag;
	}

	void LoadLengthCounter(uint8_t value)
	{
		if(_enabled) {
			_lengthCounterReloadValue = _lcLookupTable[value];
			_lengthCounterPreviousValue = _lengthCounter;
			_console->GetApu()->SetNeedToRun();
		}
	}
	
	ApuLengthCounter(AudioChannel channel, NesConsole* console)
	{
		_console = console;
	}
	
	void Reset(bool softReset)
	{
		if(softReset) {
			_enabled = false;
			if(_channel != AudioChannel::Triangle) {
				//"At reset, length counters should be enabled, triangle unaffected"
				_lengthCounterHalt = false;
				_lengthCounter = 0;
				_newHaltValue = false;
				_lengthCounterReloadValue = 0;
				_lengthCounterPreviousValue = 0;			
			}
		} else {
			_enabled = false;
			_lengthCounterHalt = false;
			_lengthCounter = 0;
			_newHaltValue = false;
			_lengthCounterReloadValue = 0;
			_lengthCounterPreviousValue = 0;		
		}
	}

	void Serialize(Serializer &s) override
	{
		SV(_enabled); SV(_lengthCounterHalt); SV(_newHaltValue); SV(_lengthCounter); SV(_lengthCounterPreviousValue); SV(_lengthCounterReloadValue);
	}

	bool GetStatus()
	{
		return _lengthCounter > 0;
	}

	bool IsHalted()
	{
		return _lengthCounterHalt;
	}
	
	void ReloadCounter()
	{
		if(_lengthCounterReloadValue) {
			if(_lengthCounter == _lengthCounterPreviousValue) {
				_lengthCounter = _lengthCounterReloadValue;
			}
			_lengthCounterReloadValue = 0;
		}

		_lengthCounterHalt = _newHaltValue;
	}

	void TickLengthCounter()
	{
		if(_lengthCounter > 0 && !_lengthCounterHalt) {
			_lengthCounter--;
		}
	}

	void SetEnabled(bool enabled)
	{
		if(!enabled) {
			_lengthCounter = 0;
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
		state.Counter = _lengthCounter;
		state.Halt = _lengthCounterHalt;
		state.ReloadValue = _lengthCounterReloadValue;
		return state;
	}
};
