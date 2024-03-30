#include "pch.h"
#include "Gameboy/APU/GbNoiseChannel.h"
#include "Gameboy/APU/GbApu.h"
#include "Gameboy/APU/GbEnvelope.h"

GbNoiseChannel::GbNoiseChannel(GbApu* apu)
{
	_apu = apu;
}

GbNoiseState& GbNoiseChannel::GetState()
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
	_state.Timer = GetPeriod();
}

void GbNoiseChannel::ResetLengthCounter()
{
	_state.Length = 0;
}

void GbNoiseChannel::ClockLengthCounter()
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

void GbNoiseChannel::UpdateOutput()
{
	_state.Output = ((_state.ShiftRegister & 0x01) ^ 0x01) * _state.Volume;
}

void GbNoiseChannel::ClockEnvelope()
{
	GbEnvelope::ClockEnvelope(_state, *this);
}

uint8_t GbNoiseChannel::GetRawOutput()
{
	return (!_state.EnvRaiseVolume && _state.EnvVolume == 0) ? 0 : _state.Output;
}

double GbNoiseChannel::GetOutput()
{
	//"If a DAC is enabled, the digital range $0 to $F is linearly translated to the analog range -1 to 1, 
	//in arbitrary units. Importantly, the slope is negative: “digital 0” maps to “analog 1”, not “analog -1”."	

	//Return -7 to 7 "analog" range (higher digital value = lower analog value)
	return (7 - (int8_t)_state.Output) * (double)_dac.GetDacVolume() / 100;
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
	_dac.Exec(clocksToRun, _state.EnvVolume || _state.EnvRaiseVolume);

	if(!_state.Enabled) {
		return;
	}

	if(_state.PeriodShift >= 14) {
		//Using a noise channel clock shift of 14 or 15 results in the LFSR receiving no clocks.
		return;
	}

	_state.Timer -= clocksToRun;

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

		UpdateOutput();
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

		case 2: {
			GbEnvelope::WriteRegister(_state, value, *this);
			break;
		}

		case 3:
			_state.PeriodShift = (value & 0xF0) >> 4;
			_state.ShortWidthMode = (value & 0x08) != 0;
			_state.Divisor = value & 0x07;
			break;

		case 4: {
			if(value & 0x80) {
				//Writing a value to NRx4 with bit 7 set causes the following things to occur :

				//Based on SameSuite's channel_4_delay results:
				// -When period =  8, 15 cpu cycles (60 clocks) (for the output to change)
				//       period = 16, 28 cpu cycles (112 clocks)
				//       period = 32, 54 cpu cycles (216 clocks)
				// -It takes 7 ticks for the first 0 to appear on bit 0 of the LFSR (in 7-bit mode)
				//    => Which would give the following clock counts: 7 * 8 = 56, 7 * 16 = 112, 7 * 32 = 224
				//    => This doesn't match (small periods are too fast, long period are too slow)
				//    => If the first period duration is halved and a constant 8 is added (like square channel timing), this becomes:
				//       8+4+6*8=60, 8+8+6*16=112, 8+16+6*32=216, which matches the results and passes
				_state.Timer = (GetPeriod() / 2 + 8) + (_apu->IsOddApuCycle() ? 2 : 0);
				if(_state.Enabled) {
					//SameSuite's channel_4_lfsr_restart and channel_4_lfsr_restart_fast tests seem to
					//expect the channel to take an extra tick when restarted while it's still running
					_state.Timer += 4;
				}

				if(_state.Divisor && !(_apu->GetElapsedApuCycles() & 0x04)) {
					//Based on SameSuite's channel_4_frequency_alignment:
					// -When divisor isn't 0, the timing sometimes changes (+/- 1 cpu cycle)
					//   -> But, when the write to NR44 is done 1 cpu cycle later, the timing is always "normal"
					//      So this can only happen every other CPU cycle?
					// -When divisor is 2, 3 or 4, the clock happens earlier
					// -When divisor is 1, the clock happens later
					//   -> So it seems like only divisor 1 occurs later and 2/3/4 occur earlier (5/6/7 are unused by the tests)
					// -channel_4_equivalent_frequencies test shows the same pattern (2/4 later, 1 earlier, 0 in the middle)
					_state.Timer += (_state.Divisor == 1) ? 4 : -4;
				}

				//Channel is enabled, if volume is not 0 or raise volume flag is set
				_state.Enabled = _state.EnvRaiseVolume || _state.EnvVolume > 0;

				//Noise channel's LFSR bits are all set to 1.
				_state.ShiftRegister = 0x7FFF;

				//If length counter is zero, it is set to 64 (256 for wave channel).
				if(_state.Length == 0) {
					_state.Length = 64;
					_state.LengthEnabled = false;
				}

				//Volume envelope timer is reloaded with period.
				_state.EnvTimer = _state.EnvPeriod;
				_state.EnvStopped = false;

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
	SV(_state.Volume); SV(_state.EnvVolume); SV(_state.EnvRaiseVolume); SV(_state.EnvPeriod); SV(_state.EnvTimer); SV(_state.EnvStopped);
	SV(_state.ShiftRegister); SV(_state.PeriodShift); SV(_state.Divisor); SV(_state.ShortWidthMode);
	SV(_state.Length); SV(_state.LengthEnabled); SV(_state.Enabled); SV(_state.Timer); SV(_state.Output);
	SV(_dac);
}
