#include "pch.h"
#include "Gameboy/APU/GbSquareChannel.h"
#include "Gameboy/APU/GbApu.h"
#include "Gameboy/APU/GbEnvelope.h"

GbSquareChannel::GbSquareChannel(GbApu* apu)
{
	_apu = apu;
}

GbSquareState& GbSquareChannel::GetState()
{
	return _state;
}

bool GbSquareChannel::Enabled()
{
	return _state.Enabled;
}

void GbSquareChannel::Disable()
{
	uint8_t len = _state.Length;
	_state = {};
	_state.Length = len;
	_state.Timer = 2048 * 4;
}

void GbSquareChannel::ResetLengthCounter()
{
	_state.Length = 0;
}

void GbSquareChannel::ClockSweepUnit()
{
	if(!_state.SweepEnabled) {
		return;
	}

	if(--_state.SweepTimer == 0) {
		_state.SweepTimer = _state.SweepPeriod ? _state.SweepPeriod : 8;

		if(_state.SweepPeriod == 0) {
			return;
		}

		//"When it generates a clock and the sweep's internal enabled flag is set and the sweep period is not zero, a new frequency is calculated and the overflow"
		uint16_t newFreq = GetSweepTargetFrequency();
		if(_state.SweepNegate) {
			_state.SweepNegateCalcDone = true;
		}

		if(newFreq >= 2048) {
			_state.Enabled = false;
			_state.SweepEnabled = false;
			_state.Output = 0;
		} else {
			//"If the new frequency is 2047 or less and the sweep shift is not zero, this new frequency is written back to the shadow frequency and square 1's frequency in NR13 and NR14,"
			if(_state.SweepShift) {
				_state.Frequency = newFreq;
				_state.SweepFreq = newFreq;

				//8 cpu cycles later, the next frequency is checked
				//This is checked by SameSuite's channel_1_sweep test
				_state.SweepUpdateDelay = 8 * 4;
			}
		}
	}
}

uint16_t GbSquareChannel::GetSweepTargetFrequency()
{
	uint16_t shiftResult = (_state.SweepFreq >> _state.SweepShift);
	if(_state.SweepNegate) {
		return _state.SweepFreq - shiftResult;
	} else {
		return _state.SweepFreq + shiftResult;
	}
}

void GbSquareChannel::ClockLengthCounter()
{
	if(_state.LengthEnabled && _state.Length > 0) {
		_state.Length--;
		if(_state.Length == 0) {
			//"Length becoming 0 should clear status"
			_state.Enabled = false;
			_state.Output = 0;
		}
	}
}

void GbSquareChannel::UpdateOutput()
{
	_state.Output = _dutySequences[_state.Duty][(_state.DutyPos - 1) & 0x07] * _state.Volume;
}

void GbSquareChannel::ClockEnvelope()
{
	GbEnvelope::ClockEnvelope(_state, *this);
}

uint8_t GbSquareChannel::GetRawOutput()
{
	return (!_state.EnvRaiseVolume && _state.EnvVolume == 0) ? 0 : _state.Output;
}

double GbSquareChannel::GetOutput()
{
	//"If a DAC is enabled, the digital range $0 to $F is linearly translated to the analog range -1 to 1, 
	//in arbitrary units. Importantly, the slope is negative: “digital 0” maps to “analog 1”, not “analog -1”."	
	
	//Return -7 to 7 "analog" range (higher digital value = lower analog value)
	return (7 - (int8_t)_state.Output) * (double)_dac.GetDacVolume() / 100;
}

void GbSquareChannel::Exec(uint32_t clocksToRun)
{
	_dac.Exec(clocksToRun, _state.EnvVolume || _state.EnvRaiseVolume);

	if(!_state.Enabled) {
		return;
	}

	if(_state.SweepUpdateDelay) {
		if(_state.SweepUpdateDelay > clocksToRun) {
			_state.SweepUpdateDelay -= clocksToRun;
		} else {
			_state.SweepUpdateDelay = 0;
			
			//"If the new frequency is 2047 or less and the sweep shift is not zero, this new frequency is written back to the shadow frequency and square 1's frequency in NR13 and NR14,"
			if(_state.SweepShift) {
				uint16_t newFreq = GetSweepTargetFrequency();
				if(newFreq >= 2048) {
					//"then frequency calculation and overflow check are run AGAIN immediately using this new value, but this second new frequency is not written back."
					_state.Enabled = false;
					_state.SweepEnabled = false;
					_state.Output = 0;
					return;
				}
			}
		}
	}

	_state.Timer -= clocksToRun;

	if(_state.Timer == 0) {
		_state.Timer = (2048 - _state.Frequency) * 4;
		_state.DutyPos = (_state.DutyPos + 1) & 0x07;
		UpdateOutput();
	}
}

uint8_t GbSquareChannel::Read(uint16_t addr)
{
	constexpr uint8_t openBusBits[5] = { 0x80, 0x3F, 0x00, 0xFF, 0xBF };

	uint8_t value = 0;
	switch(addr) {
		case 0:
			value = (
				(_state.SweepPeriod << 4) |
				(_state.SweepNegate ? 0x08 : 0) |
				_state.SweepShift
			);
			break;

		case 1: value = _state.Duty << 6; break;

		case 2:
			value = (
				(_state.EnvVolume << 4) |
				(_state.EnvRaiseVolume ? 0x08 : 0) |
				_state.EnvPeriod
			);
			break;

		case 4: value = _state.LengthEnabled ? 0x40 : 0; break;
	}

	return value | openBusBits[addr];
}

void GbSquareChannel::Write(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0:
			_state.SweepShift = value & 0x07;
			_state.SweepNegate = (value & 0x08) != 0;
			_state.SweepPeriod = (value & 0x70) >> 4;

			if(!_state.SweepNegate && _state.SweepNegateCalcDone) {
				//Disabling negate mode after a sweep freq calculation was performed
				//while negate mode was enabled will disable the channel
				//Required for sweep-details tests 4, 5 and 6
				_state.Enabled = false;
				_state.Output = 0;
			}
			break;

		case 1:
			_state.Length = 64 - (value & 0x3F);
			_state.Duty = (value & 0xC0) >> 6;
			break;

		case 2: {
			GbEnvelope::WriteRegister(_state, value, *this);
			UpdateOutput();
			break;
		}

		case 3:
			_state.Frequency = (_state.Frequency & 0x700) | value;
			break;

		case 4: {
			_state.Frequency = (_state.Frequency & 0xFF) | ((value & 0x07) << 8);

			if(value & 0x80) {
				//"Writing a value to NRx4 with bit 7 set causes the following things to occur :"

				//Frequency timer is reloaded with period.
				//"When triggering a square channel, the low two bits of the frequency timer are NOT modified."
				_state.Timer = (((2048 - _state.Frequency) * 4) + 8) + (_apu->IsOddApuCycle() ? 2 : 0);
				if(_state.Enabled) {
					//Contrary to the noise channel, it looks like SameSuite's channel_1_restart test expects
					//the channel to take one tick *less* when restarted while it's still running?
					_state.Timer -= 4;
				}

				//"Channel volume is reloaded from NRx2."
				_state.Volume = _state.EnvVolume;

				if(_state.Enabled) {
					//Updating the output here is needed to pass the channel_1_restart_nrx2_glitch test
					//Only do this if the channel was already enabled, otherwise other tests fail
					UpdateOutput();
				}

				//"Channel is enabled, if volume is not 0 or raise volume flag is set"
				_state.Enabled = _state.EnvRaiseVolume || _state.EnvVolume > 0;

				//"If length counter is zero, it is set to 64 (256 for wave channel)."
				if(_state.Length == 0) {
					_state.Length = 64;
					_state.LengthEnabled = false;
				}

				//"Volume envelope timer is reloaded with period."
				_state.EnvTimer = _state.EnvPeriod;
				_state.EnvStopped = false;

				//Sweep-related
				//"During a trigger event, several things occur:"
				//"Square 1's frequency is copied to the shadow register."
				_state.SweepFreq = _state.Frequency;

				//"The sweep timer is reloaded."
				//"The volume envelope and sweep timers treat a period of 0 as 8."
				_state.SweepTimer = _state.SweepPeriod ? _state.SweepPeriod : 8;
				_state.SweepNegateCalcDone = false;

				//"The internal enabled flag is set if either the sweep period or shift are non-zero, cleared otherwise.
				_state.SweepEnabled = _state.SweepPeriod > 0 || _state.SweepShift > 0;

				//"If the sweep shift is non-zero, frequency calculation and the overflow check are performed immediately."
				if(_state.SweepShift > 0) {
					//After ~8 cpu cycles, the next frequency is checked (which can disable the channel)
					_state.SweepUpdateDelay = 8 * 4 + 2;
					if(_state.SweepNegate) {
						_state.SweepNegateCalcDone = true;
					}
				}
			}

			_apu->ProcessLengthEnableFlag(value, _state.Length, _state.LengthEnabled, _state.Enabled);
			break;
		}
	}
}

void GbSquareChannel::Serialize(Serializer& s)
{
	SV(_state.SweepPeriod); SV(_state.SweepNegate); SV(_state.SweepShift); SV(_state.SweepTimer); SV(_state.SweepEnabled); SV(_state.SweepFreq);
	SV(_state.Volume); SV(_state.EnvVolume); SV(_state.EnvRaiseVolume); SV(_state.EnvPeriod); SV(_state.EnvTimer); SV(_state.Duty); SV(_state.Frequency);
	SV(_state.Length); SV(_state.LengthEnabled); SV(_state.Enabled); SV(_state.Timer); SV(_state.DutyPos); SV(_state.Output);
	SV(_state.SweepNegateCalcDone); SV(_state.EnvStopped);
	SV(_state.SweepUpdateDelay);
	SV(_dac);
}
