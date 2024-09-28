#pragma once
#include "pch.h"
#include "NES/APU/ApuEnvelope.h"
#include "NES/APU/ApuTimer.h"
#include "NES/APU/NesApu.h"
#include "NES/NesConstants.h"
#include "NES/NesConsole.h"
#include "NES/INesMemoryHandler.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Serializer.h"

class SquareChannel : public INesMemoryHandler, public ISerializable
{
protected:
	static constexpr uint8_t _dutySequences[4][8] = {
		{ 0, 0, 0, 0, 0, 0, 0, 1 },
		{ 0, 0, 0, 0, 0, 0, 1, 1 },
		{ 0, 0, 0, 0, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1, 1, 0, 0 }
	};

	NesConsole* _console = nullptr;
	ApuEnvelope _envelope;
	ApuTimer _timer;

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
		_timer.SetPeriod((_realPeriod * 2) + 1);
		UpdateTargetPeriod();
	}

	void UpdateOutput()
	{
		if(IsMuted()) {
			_timer.AddOutput(0);
		} else {
			_timer.AddOutput(_dutySequences[_duty][_dutyPos] * _envelope.GetVolume());
		}
	}

public:
	SquareChannel(AudioChannel channel, NesConsole* console, bool isChannel1) : _envelope(channel, console), _timer(channel, console->GetSoundMixer())
	{
		_console = console;
		_isChannel1 = isChannel1;
	}

	void Run(uint32_t targetCycle)
	{
		while(_timer.Run(targetCycle)) {
			_dutyPos = (_dutyPos - 1) & 0x07;
			UpdateOutput();
		}
	}

	void Reset(bool softReset)
	{
		_envelope.Reset(softReset);
		_timer.Reset(softReset);

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
		SV(_realPeriod); SV(_duty); SV(_dutyPos); SV(_sweepEnabled); SV(_sweepPeriod); SV(_sweepNegate); SV(_sweepShift); SV(_reloadSweep); SV(_sweepDivider); SV(_sweepTargetPeriod);
		SV(_timer);
		SV(_envelope);
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
				_envelope.InitializeEnvelope(value);

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
				_envelope.LengthCounter.LoadLengthCounter(value >> 3);

				SetPeriod((_realPeriod & 0xFF) | ((value & 0x07) << 8));

				//The sequencer is restarted at the first value of the current sequence.
				_dutyPos = 0;

				//The envelope is also restarted.
				_envelope.ResetEnvelope();
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

	void TickEnvelope()
	{
		_envelope.TickEnvelope();
	}

	void TickLengthCounter()
	{
		_envelope.LengthCounter.TickLengthCounter();
	}

	void ReloadLengthCounter()
	{
		_envelope.LengthCounter.ReloadCounter();
	}

	void EndFrame()
	{
		_timer.EndFrame();
	}

	void SetEnabled(bool enabled)
	{
		_envelope.LengthCounter.SetEnabled(enabled);
	}

	bool GetStatus()
	{
		return _envelope.LengthCounter.GetStatus();
	}

	uint8_t GetOutput()
	{
		return _timer.GetLastOutput();
	}

	ApuSquareState GetState()
	{
		ApuSquareState state;
		state.Duty = _duty;
		state.DutyPosition = _dutyPos;
		state.Enabled = _envelope.LengthCounter.IsEnabled();
		state.Envelope = _envelope.GetState();
		state.Frequency = NesConstants::GetClockRate(NesApu::GetApuRegion(_console)) / 16.0 / (_realPeriod + 1);
		state.LengthCounter = _envelope.LengthCounter.GetState();
		state.OutputVolume = _timer.GetLastOutput();
		state.Period = _realPeriod;
		state.Timer = _timer.GetTimer() / 2;
		state.SweepEnabled = _sweepEnabled;
		state.SweepNegate = _sweepNegate;
		state.SweepPeriod = _sweepPeriod;
		state.SweepShift = _sweepShift;
		return state;
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		return 0;
	}
};