#pragma once
#include "../stdafx.h"
#include "BaseApuChannel.h"
#include "NES/NesConsole.h"
#include "NesApu.h"

class ApuLengthCounter : public BaseApuChannel
{
private:
	uint8_t _lcLookupTable[32] = { 10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60, 10, 14, 12, 26, 14, 12, 16, 24, 18, 48, 20, 96, 22, 192, 24, 72, 26, 16, 28, 32, 30 };
	bool _newHaltValue;

protected:
	bool _enabled = false;
	bool _lengthCounterHalt;
	uint8_t _lengthCounter;
	uint8_t _lengthCounterReloadValue;
	uint8_t _lengthCounterPreviousValue;

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
	
public:
	ApuLengthCounter(AudioChannel channel, shared_ptr<NesConsole> console, NesSoundMixer* mixer) : BaseApuChannel(channel, console, mixer)
	{
	}
	
	virtual void Reset(bool softReset) override
	{
		BaseApuChannel::Reset(softReset);

		if(softReset) {
			_enabled = false;
			if(GetChannel() != AudioChannel::Triangle) {
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
		BaseApuChannel::Serialize(s);

		s.Stream(_enabled, _lengthCounterHalt, _newHaltValue, _lengthCounter, _lengthCounterPreviousValue, _lengthCounterReloadValue);
	}

	bool GetStatus() override
	{
		return _lengthCounter > 0;
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

	ApuLengthCounterState GetState()
	{
		ApuLengthCounterState state;
		state.Counter = _lengthCounter;
		state.Halt = _lengthCounterHalt;
		state.ReloadValue = _lengthCounterReloadValue;
		return state;
	}
};
