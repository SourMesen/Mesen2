#pragma once
#include "stdafx.h"
#include "ApuEnvelope.h"
#include "NES/NesConsole.h"

class SquareChannel : public ApuEnvelope
{
protected:
	const uint8_t _dutySequences[4][8] = {
		{ 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 0, 0, 0, 0, 0, 0, 1, 1 },
		{ 0, 0, 0, 0, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 0, 0 }
	};

	bool _isChannel1 = false;
	bool _isMmc5Square = false;

	uint8_t _duty = 0;
	uint8_t _dutyPos = 0;
	
	bool _sweepEnabled = false;
	uint8_t _sweepPeriod = 0;
	bool _sweepNegate = false;
	uint8_t _sweepShift = 0;
	bool _reloadSweep = false;
	uint8_t _sweepDivider = 0;
	uint32_t _sweepTargetPeriod = 0;
	uint16_t _realPeriod = 0;
	
	bool IsMuted()
	{
		//A period of t < 8, either set explicitly or via a sweep period update, silences the corresponding pulse channel.
		return _realPeriod < 8 || (!_sweepNegate && _sweepTargetPeriod > 0x7FF);
	}

	virtual void InitializeSweep(uint8_t regValue)
	{
		_sweepEnabled = (regValue & 0x80) == 0x80;
		_sweepNegate = (regValue & 0x08) == 0x08;

		//The divider's period is set to P + 1 
		_sweepPeriod = ((regValue & 0x70) >> 4) + 1;
		_sweepShift = (regValue & 0x07);

		UpdateTargetPeriod();

		//Side effects: Sets the reload flag 
		_reloadSweep = true;
	}

	void UpdateTargetPeriod()
	{
		uint16_t shiftResult = (_realPeriod >> _sweepShift);
		if(_sweepNegate) {
			_sweepTargetPeriod = _realPeriod - shiftResult;
			if(_isChannel1) {
				// As a result, a negative sweep on pulse channel 1 will subtract the shifted period value minus 1
				_sweepTargetPeriod--;
			}
		} else {
			_sweepTargetPeriod = _realPeriod + shiftResult;
		}
	}

	void SetPeriod(uint16_t newPeriod)
	{
		_realPeriod = newPeriod;
		_period = (_realPeriod * 2) + 1;
		UpdateTargetPeriod();
	}

	void UpdateOutput()
	{
		if(IsMuted()) {
			AddOutput(0);
		} else {
			AddOutput(_dutySequences[_duty][_dutyPos] * GetVolume());
		}
	}

protected:
	void Clock() override
	{
		_dutyPos = (_dutyPos - 1) & 0x07;
		UpdateOutput();
	}

public:
	SquareChannel(AudioChannel channel, shared_ptr<NesConsole> console, NesSoundMixer *mixer, bool isChannel1) : ApuEnvelope(channel, console, mixer)
	{
		_isChannel1 = isChannel1;
	}

	virtual void Reset(bool softReset) override
	{
		ApuEnvelope::Reset(softReset);
		
		_duty = 0;
		_dutyPos = 0;

		_realPeriod = 0;
	
		_sweepEnabled = false;
		_sweepPeriod = 0;
		_sweepNegate = false;
		_sweepShift = 0;
		_reloadSweep = false;
		_sweepDivider = 0;
		_sweepTargetPeriod = 0;
		UpdateTargetPeriod();
	}

	void Serialize(Serializer& s) override
	{
		ApuEnvelope::Serialize(s);

		s.Stream(_realPeriod, _duty, _dutyPos, _sweepEnabled, _sweepPeriod, _sweepNegate, _sweepShift, _reloadSweep, _sweepDivider, _sweepTargetPeriod);
	}

	void GetMemoryRanges(MemoryRanges &ranges) override
	{
		if(_isChannel1) {
			ranges.AddHandler(MemoryOperation::Write, 0x4000, 0x4003);
		} else {
			ranges.AddHandler(MemoryOperation::Write, 0x4004, 0x4007);
		}
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		_console->GetApu()->Run();
		switch(addr & 0x03) {
			case 0:		//4000 & 4004
				InitializeLengthCounter((value & 0x20) == 0x20);
				InitializeEnvelope(value);

				_duty = (value & 0xC0) >> 6;
				if(_console->GetNesConfig().SwapDutyCycles) {
					_duty = ((_duty & 0x02) >> 1) | ((_duty & 0x01) << 1);
				}
				break;

			case 1:		//4001 & 4005
				InitializeSweep(value);
				break;

			case 2:		//4002 & 4006
				SetPeriod((_realPeriod & 0x0700) | value);
				break;

			case 3:		//4003 & 4007
				LoadLengthCounter(value >> 3);

				SetPeriod((_realPeriod & 0xFF) | ((value & 0x07) << 8));

				//The sequencer is restarted at the first value of the current sequence.
				_dutyPos = 0;

				//The envelope is also restarted.
				ResetEnvelope();
				break;
		}
		
		if(!_isMmc5Square) {
			UpdateOutput();
		}
	}

	void TickSweep()
	{
		_sweepDivider--;
		if(_sweepDivider == 0) {
			if(_sweepShift > 0 && _sweepEnabled && _realPeriod >= 8 && _sweepTargetPeriod <= 0x7FF) {
				SetPeriod(_sweepTargetPeriod);
			}
			_sweepDivider = _sweepPeriod;
		}

		if(_reloadSweep) {
			_sweepDivider = _sweepPeriod;
			_reloadSweep = false;
		}
	}

	ApuSquareState GetState()
	{
		ApuSquareState state;
		state.Duty = _duty;
		state.DutyPosition = _dutyPos;
		state.Enabled = _enabled;
		state.Envelope = ApuEnvelope::GetState();
		state.Frequency = _console->GetCpu()->GetClockRate(GetNesModel()) / 16.0 / (_realPeriod + 1);
		state.LengthCounter = ApuLengthCounter::GetState();
		state.OutputVolume = _lastOutput;
		state.Period = _realPeriod;
		state.Timer = _timer / 2;
		state.SweepEnabled = _sweepEnabled;
		state.SweepNegate = _sweepNegate;
		state.SweepPeriod = _sweepPeriod;
		state.SweepShift = _sweepShift;
		return state;
	}
};