#pragma once
#include "../stdafx.h"
#include "NesApu.h"
#include "ApuEnvelope.h"
#include "NesCpu.h"
#include "NesConsole.h"

class NoiseChannel : public ApuEnvelope
{
private:	
	const uint16_t _noisePeriodLookupTableNtsc[16] = { 4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068 };
	const uint16_t _noisePeriodLookupTablePal[16] = { 4, 8, 14, 30, 60, 88, 118, 148, 188, 236, 354, 472, 708,  944, 1890, 3778 };

	//On power-up, the shift register is loaded with the value 1.
	uint16_t _shiftRegister = 1;
	bool _modeFlag = false;

	bool IsMuted()
	{
		//The mixer receives the current envelope volume except when Bit 0 of the shift register is set, or The length counter is zero
		return (_shiftRegister & 0x01) == 0x01;
	}

protected:
	void Clock() override
	{
		//Feedback is calculated as the exclusive-OR of bit 0 and one other bit: bit 6 if Mode flag is set, otherwise bit 1.
		bool mode = _console->GetNesConfig().DisableNoiseModeFlag ? false : _modeFlag;

		uint16_t feedback = (_shiftRegister & 0x01) ^ ((_shiftRegister >> (mode ? 6 : 1)) & 0x01);
		_shiftRegister >>= 1;
		_shiftRegister |= (feedback << 14);

		if(IsMuted()) {
			AddOutput(0);
		} else {
			AddOutput(GetVolume()); 
		}
	}

public:
	NoiseChannel(AudioChannel channel, shared_ptr<NesConsole> console, NesSoundMixer* mixer) : ApuEnvelope(channel, console, mixer)
	{
	}

	virtual void Reset(bool softReset) override
	{
		ApuEnvelope::Reset(softReset);
		
		_period = (GetNesModel() == NesModel::NTSC ? _noisePeriodLookupTableNtsc : _noisePeriodLookupTablePal)[0] - 1;
		_shiftRegister = 1;
		_modeFlag = false;
	}

	void Serialize(Serializer& s) override
	{
		ApuEnvelope::Serialize(s);

		s.Stream(_shiftRegister, _modeFlag);
	}

	void GetMemoryRanges(MemoryRanges &ranges) override
	{
		ranges.AddHandler(MemoryOperation::Write, 0x400C, 0x400F);
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		_console->GetApu()->Run();

		switch(addr & 0x03) {
			case 0:		//400C
				InitializeLengthCounter((value & 0x20) == 0x20);
				InitializeEnvelope(value);
				break;

			case 2:		//400E
				_period = (GetNesModel() == NesModel::NTSC ? _noisePeriodLookupTableNtsc : _noisePeriodLookupTablePal)[value & 0x0F] - 1;
				_modeFlag = (value & 0x80) == 0x80;
				break;

			case 3:		//400F
				LoadLengthCounter(value >> 3);

				//The envelope is also restarted.
				ResetEnvelope();
				break;
		}
	}

	ApuNoiseState GetState()
	{
		ApuNoiseState state;
		state.Enabled = _enabled;
		state.Envelope = ApuEnvelope::GetState();
		state.Frequency = (double)_console->GetCpu()->GetClockRate(GetNesModel()) / (_period + 1) / (_modeFlag ? 93 : 1);
		state.LengthCounter = ApuLengthCounter::GetState();
		state.ModeFlag = _modeFlag;
		state.OutputVolume = _lastOutput;
		state.Period = _period;
		state.Timer = _timer;
		state.ShiftRegister = _shiftRegister;
		return state;
	}
};