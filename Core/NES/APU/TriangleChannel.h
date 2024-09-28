#pragma once
#include "pch.h"
#include "NES/NesConsole.h"
#include "NES/NesConstants.h"
#include "NES/APU/ApuTimer.h"
#include "NES/APU/ApuLengthCounter.h"
#include "NES/INesMemoryHandler.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class TriangleChannel : public INesMemoryHandler, public ISerializable
{
private:
	static constexpr uint8_t _sequence[32] = { 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };

	NesConsole* _console;
	ApuLengthCounter _lengthCounter;
	ApuTimer _timer;

	uint8_t _linearCounter = 0;
	uint8_t _linearCounterReload = 0;
	bool _linearReloadFlag = false;
	bool _linearControlFlag = false;

	uint8_t _sequencePosition = 0;

public:
	TriangleChannel(NesConsole* console) : _lengthCounter(AudioChannel::Triangle, console), _timer(AudioChannel::Triangle, console->GetSoundMixer())
	{
		_console = console;
	}

	void Run(uint32_t targetCycle)
	{
		while(_timer.Run(targetCycle)) {
			//The sequencer is clocked by the timer as long as both the linear counter and the length counter are nonzero. 
			if(_lengthCounter.GetStatus() && _linearCounter > 0) {
				_sequencePosition = (_sequencePosition + 1) & 0x1F;

				if(_timer.GetPeriod() >= 2 || !_console->GetNesConfig().SilenceTriangleHighFreq) {
					//Disabling the triangle channel when period is < 2 removes "pops" in the audio that are caused by the ultrasonic frequencies
					//This is less "accurate" in terms of emulation, so this is an option (disabled by default)
					_timer.AddOutput(_sequence[_sequencePosition]);
				}
			}
		}
	}

	void Reset(bool softReset)
	{
		_timer.Reset(softReset);
		_lengthCounter.Reset(softReset);

		_linearCounter = 0;
		_linearCounterReload = 0;
		_linearReloadFlag = false;
		_linearControlFlag = false;

		_sequencePosition = 0;
	}

	void Serialize(Serializer& s) override
	{
		SV(_linearCounter); SV(_linearCounterReload); SV(_linearReloadFlag); SV(_linearControlFlag); SV(_sequencePosition);
		SV(_timer);
		SV(_lengthCounter);
	}

	void GetMemoryRanges(MemoryRanges &ranges) override
	{
		ranges.AddHandler(MemoryOperation::Write, 0x4008, 0x400B);
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		_console->GetApu()->Run();

		switch(addr & 0x03) {
			case 0:		//4008
				_linearControlFlag = (value & 0x80) == 0x80;
				_linearCounterReload = value & 0x7F;

				_lengthCounter.InitializeLengthCounter(_linearControlFlag);
				break;

			case 2:		//400A
				_timer.SetPeriod((_timer.GetPeriod() & 0xFF00) | value);
				break;

			case 3:		//400B
				_lengthCounter.LoadLengthCounter(value >> 3);

				_timer.SetPeriod((_timer.GetPeriod() & 0xFF) | ((value & 0x07) << 8));

				//Side effects 	Sets the linear counter reload flag 
				_linearReloadFlag = true;
				break;
		}
	}

	void TickLinearCounter()
	{
		if(_linearReloadFlag) {
			_linearCounter = _linearCounterReload;
		} else if(_linearCounter > 0) {
			_linearCounter--;
		}

		if(!_linearControlFlag) {
			_linearReloadFlag = false;
		}
	}
	
	void TickLengthCounter()
	{
		_lengthCounter.TickLengthCounter();
	}

	void ReloadLengthCounter()
	{
		_lengthCounter.ReloadCounter();
	}

	void EndFrame()
	{
		_timer.EndFrame();
	}

	void SetEnabled(bool enabled)
	{
		_lengthCounter.SetEnabled(enabled);
	}

	bool GetStatus()
	{
		return _lengthCounter.GetStatus();
	}

	uint8_t GetOutput()
	{
		return _timer.GetLastOutput();
	}

	ApuTriangleState GetState()
	{
		ApuTriangleState state;
		state.Enabled = _lengthCounter.IsEnabled();
		state.Frequency = NesConstants::GetClockRate(NesApu::GetApuRegion(_console)) / 32.0 / (_timer.GetPeriod() + 1);
		state.LengthCounter = _lengthCounter.GetState();
		state.OutputVolume = _timer.GetLastOutput();
		state.Period = _timer.GetPeriod();
		state.Timer = _timer.GetTimer();
		state.SequencePosition = _sequencePosition;
		state.LinearCounterReload = _linearCounterReload;
		state.LinearCounter = _linearCounter;
		state.LinearReloadFlag = _linearReloadFlag;
		return state;
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		return 0;
	}
};