#include "stdafx.h"
#include "GbNoiseChannel.h"
#include "GbApu.h"

GbNoiseChannel::GbNoiseChannel(GbApu* apu)
{
	_apu = apu;
}

GbNoiseState GbNoiseChannel::GetState()
{
	return _state;
}

bool GbNoiseChannel::Enabled()
{
	return _state.Enabled;
}

void GbNoiseChannel::Disable()
{
	uint8_t len = _state.Length;
	_state = {};
	_state.Length = len;
}

void GbNoiseChannel::ClockLengthCounter()
{
	if(_state.LengthEnabled && _state.Length > 0) {
		_state.Length--;
		if(_state.Length == 0) {
			//"Length becoming 0 should clear status"
			_state.Enabled = false;
		}
	}
}

void GbNoiseChannel::ClockEnvelope()
{
	if(_state.EnvTimer > 0) {
		_state.EnvTimer--;

		if(_state.EnvTimer == 0) {
			if(_state.EnvRaiseVolume && _state.Volume < 0x0F) {
				_state.Volume++;
			} else if(!_state.EnvRaiseVolume && _state.Volume > 0) {
				_state.Volume--;
			}

			_state.EnvTimer = _state.EnvPeriod;
		}
	}
}

uint8_t GbNoiseChannel::GetOutput()
{
	return _state.Output;
}

uint32_t GbNoiseChannel::GetPeriod()
{
	if(_state.Divisor == 0) {
		return 8 << _state.PeriodShift;
	} else {
		return (16 * _state.Divisor) << _state.PeriodShift;
	}
}

void GbNoiseChannel::Exec(uint32_t clocksToRun)
{
	if(_state.PeriodShift >= 14) {
		//Using a noise channel clock shift of 14 or 15 results in the LFSR receiving no clocks.
		return;
	}

	_state.Timer -= clocksToRun;

	if(_state.Enabled) {
		_state.Output = ((_state.ShiftRegister & 0x01) ^ 0x01) * _state.Volume;
	} else {
		_state.Output = 0;
	}

	if(_state.Timer == 0) {
		_state.Timer = GetPeriod();

		//When clocked by the frequency timer, the low two bits (0 and 1) are XORed, all bits are shifted right by one,
		//and the result of the XOR is put into the now-empty high bit.
		uint16_t shiftedValue = _state.ShiftRegister >> 1;
		uint8_t xorResult = (_state.ShiftRegister & 0x01) ^ (shiftedValue & 0x01);
		_state.ShiftRegister = (xorResult << 14) | shiftedValue;

		if(_state.ShortWidthMode) {
			//If width mode is 1 (NR43), the XOR result is ALSO put into bit 6 AFTER the shift, resulting in a 7-bit LFSR.
			_state.ShiftRegister &= ~0x40;
			_state.ShiftRegister |= (xorResult << 6);
		}
	}
}

uint8_t GbNoiseChannel::Read(uint16_t addr)
{
	constexpr uint8_t openBusBits[5] = { 0xFF, 0xFF, 0x00, 0x00, 0xBF };

	uint8_t value = 0;
	switch(addr) {
		case 2:
			value = (
				(_state.EnvVolume << 4) |
				(_state.EnvRaiseVolume ? 0x08 : 0) |
				_state.EnvPeriod
				);
			break;

		case 3:
			value = (
				(_state.PeriodShift << 4) |
				(_state.ShortWidthMode ? 0x08 : 0) |
				_state.Divisor
				);
			break;

		case 4: value = _state.LengthEnabled ? 0x40 : 0; break;
	}

	return value | openBusBits[addr];
}

void GbNoiseChannel::Write(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0: break;
		case 1:
			_state.Length = 64 - (value & 0x3F);
			break;

		case 2:
			_state.EnvPeriod = value & 0x07;
			_state.EnvRaiseVolume = (value & 0x08) != 0;
			_state.EnvVolume = (value & 0xF0) >> 4;

			if(!(value & 0xF8)) {
				_state.Enabled = false;
			}
			break;

		case 3:
			_state.PeriodShift = (value & 0xF0) >> 4;
			_state.ShortWidthMode = (value & 0x08) != 0;
			_state.Divisor = value & 0x07;
			break;

		case 4: {
			if(value & 0x80) {
				//Writing a value to NRx4 with bit 7 set causes the following things to occur :

				//Channel is enabled, if volume is not 0 or raise volume flag is set
				_state.Enabled = _state.EnvRaiseVolume || _state.EnvVolume > 0;

				//Frequency timer is reloaded with period.
				_state.Timer = GetPeriod();

				//Noise channel's LFSR bits are all set to 1.
				_state.ShiftRegister = 0x7FFF;

				//If length counter is zero, it is set to 64 (256 for wave channel).
				if(_state.Length == 0) {
					_state.Length = 64;
					_state.LengthEnabled = false;
				}

				//Volume envelope timer is reloaded with period.
				_state.EnvTimer = _state.EnvPeriod;

				//Channel volume is reloaded from NRx2.
				_state.Volume = _state.EnvVolume;
			}

			_apu->ProcessLengthEnableFlag(value, _state.Length, _state.LengthEnabled, _state.Enabled);
			break;
		}
	}
}

void GbNoiseChannel::Serialize(Serializer& s)
{
	s.Stream(
		_state.Volume, _state.EnvVolume, _state.EnvRaiseVolume, _state.EnvPeriod, _state.EnvTimer,
		_state.ShiftRegister, _state.PeriodShift, _state.Divisor, _state.ShortWidthMode,
		_state.Length, _state.LengthEnabled, _state.Enabled, _state.Timer, _state.Output
	);
}
