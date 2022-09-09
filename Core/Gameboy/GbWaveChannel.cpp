#include "pch.h"
#include "Gameboy/GbWaveChannel.h"
#include "Gameboy/GbApu.h"

GbWaveChannel::GbWaveChannel(GbApu* apu)
{
	_apu = apu;
}

GbWaveState GbWaveChannel::GetState()
{
	return _state;
}

bool GbWaveChannel::Enabled()
{
	return _state.Enabled;
}

void GbWaveChannel::Disable()
{
	uint16_t len = _state.Length;
	uint8_t ram[0x10];
	memcpy(ram, _state.Ram, sizeof(ram));
	_state = {};
	_state.Length = len;
	memcpy(_state.Ram, ram, sizeof(ram));
}

void GbWaveChannel::ClockLengthCounter()
{
	if(_state.LengthEnabled && _state.Length > 0) {
		_state.Length--;
		if(_state.Length == 0) {
			//"Length becoming 0 should clear status"
			_state.Enabled = false;
		}
	}
}

uint8_t GbWaveChannel::GetOutput()
{
	return _state.Output;
}

void GbWaveChannel::Exec(uint32_t clocksToRun)
{
	_state.Timer -= clocksToRun;

	//The DAC receives the current value from the upper/lower nibble of the sample buffer, shifted right by the volume control. 
	if(_state.Volume && _state.Enabled) {
		_state.Output = _state.SampleBuffer >> (_state.Volume - 1);
	} else {
		_state.Output = 0;
	}

	if(_state.Timer == 0) {
		//The wave channel's frequency timer period is set to (2048-frequency)*2.
		_state.Timer = (2048 - _state.Frequency) * 2;

		//When the timer generates a clock, the position counter is advanced one sample in the wave table,
		//looping back to the beginning when it goes past the end,
		_state.Position = (_state.Position + 1) & 0x1F;

		//then a sample is read into the sample buffer from this NEW position.
		if(_state.Position & 0x01) {
			_state.SampleBuffer = _state.Ram[_state.Position >> 1] & 0x0F;
		} else {
			_state.SampleBuffer = _state.Ram[_state.Position >> 1] >> 4;
		}
	}
}

uint8_t GbWaveChannel::Read(uint16_t addr)
{
	constexpr uint8_t openBusBits[5] = { 0x7F, 0xFF, 0x9F, 0xFF, 0xBF };

	uint8_t value = 0;
	switch(addr) {
		case 0: value = _state.DacEnabled ? 0x80 : 0; break;
		case 2: value = _state.Volume << 5; break;
		case 4: value = _state.LengthEnabled ? 0x40 : 0; break;
	}

	return value | openBusBits[addr];
}

void GbWaveChannel::Write(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0:
			_state.DacEnabled = (value & 0x80) != 0;
			_state.Enabled &= _state.DacEnabled;
			break;

		case 1:
			_state.Length = 256 - value;
			break;

		case 2:
			_state.Volume = (value & 0x60) >> 5;
			break;

		case 3:
			_state.Frequency = (_state.Frequency & 0x700) | value;
			break;

		case 4: {
			_state.Frequency = (_state.Frequency & 0xFF) | ((value & 0x07) << 8);

			if(value & 0x80) {
				//Start playback

				//Channel is enabled, if DAC is enabled
				_state.Enabled = _state.DacEnabled;

				//Frequency timer is reloaded with period.
				_state.Timer = (2048 - _state.Frequency) * 2;

				//If length counter is zero, it is set to 64 (256 for wave channel).
				if(_state.Length == 0) {
					_state.Length = 256;
					_state.LengthEnabled = false;
				}

				//Wave channel's position is set to 0 but sample buffer is NOT refilled.
				_state.Position = 0;
			}

			_apu->ProcessLengthEnableFlag(value, _state.Length, _state.LengthEnabled, _state.Enabled);
			break;
		}
	}
}

void GbWaveChannel::WriteRam(uint16_t addr, uint8_t value)
{
	_state.Ram[addr & 0x0F] = value;
}

uint8_t GbWaveChannel::ReadRam(uint16_t addr)
{
	return _state.Ram[addr & 0x0F];
}

void GbWaveChannel::Serialize(Serializer& s)
{
	SV(_state.DacEnabled); SV(_state.SampleBuffer); SV(_state.Position); SV(_state.Volume); SV(_state.Frequency);
	SV(_state.Length); SV(_state.LengthEnabled); SV(_state.Enabled); SV(_state.Timer); SV(_state.Output);
	SVArray(_state.Ram, 0x10);
}
