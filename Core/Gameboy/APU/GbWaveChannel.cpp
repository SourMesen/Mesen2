#include "pch.h"
#include "Gameboy/APU/GbWaveChannel.h"
#include "Gameboy/APU/GbApu.h"
#include "Gameboy/Gameboy.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"

GbWaveChannel::GbWaveChannel(GbApu* apu, Gameboy* gameboy)
{
	_gameboy = gameboy;
	_apu = apu;

	//"When the Game Boy is switched on (before the internal boot ROM executes),
	//the values in the wave table depend on the model. On the DMG, they are somewhat
	//random, though the particular pattern is generally the same for each individual Game Boy unit.
	//The game R-Type doesn't initialize wave RAM and thus relies on these."

	//Note: On CGB, the boot rom initalizes wave ram to 00, FF, 00, FF, etc. (so these values will be overwritten)
	if(_gameboy->GetEmulator()->GetSettings()->GetGameboyConfig().RamPowerOnState == RamState::Random) {
		//If random ram is turned on, randomize it completely instead
		_gameboy->InitializeRam(_state.Ram, 0x10);
	} else {
		//Otherwise, use a preset to ensure the audio is still audible
		constexpr uint8_t gbWaveRamDefault[0x10] = { 0x84, 0x40, 0x43, 0xAA, 0x2D, 0x78, 0x92, 0x3C, 0x60, 0x59, 0x59, 0xB0, 0x34, 0xB8, 0x2E, 0xDA };
		memcpy(_state.Ram, gbWaveRamDefault, 0x10);
	}
}

GbWaveState& GbWaveChannel::GetState()
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
	_state.Timer = 2048 * 2;
}

void GbWaveChannel::ResetLengthCounter()
{
	_state.Length = 0;
}

void GbWaveChannel::ClockLengthCounter()
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

uint8_t GbWaveChannel::GetRawOutput()
{
	//"Stopping channel 3 manually using the NR30 register affects PCM34 instantly" (SameSuite's channel_3_stop_delay test)
	return _state.Enabled ? _state.Output : 0;
}

double GbWaveChannel::GetOutput()
{
	//"If a DAC is enabled, the digital range $0 to $F is linearly translated to the analog range -1 to 1, 
	//in arbitrary units. Importantly, the slope is negative: “digital 0” maps to “analog 1”, not “analog -1”."	

	//Return -7 to 7 "analog" range (higher digital value = lower analog value)
	return (7 - (int8_t)_state.Output) * (double)_dac.GetDacVolume() / 100;
}

void GbWaveChannel::UpdateOutput()
{
	if(_state.Volume) {
		_state.Output = _state.SampleBuffer >> (_state.Volume - 1);
	} else {
		_state.Output = 0;
	}
}

void GbWaveChannel::Exec(uint32_t clocksToRun)
{
	_dac.Exec(clocksToRun, _state.DacEnabled);

	if(!_state.Enabled) {
		return;
	}

	_state.Timer -= clocksToRun;
	_allowRamAccess = false;

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

		//The DAC receives the current value from the upper/lower nibble of the sample buffer, shifted right by the volume control. 
		UpdateOutput();

		_allowRamAccess = true;
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

			//"Modifying the channel 3 shift while the channel is playing affects PCM34 instantly" (SameSuite's channel_3_shift_delay test)
			UpdateOutput();
			break;

		case 3:
			_state.Frequency = (_state.Frequency & 0x700) | value;
			break;

		case 4: {
			_state.Frequency = (_state.Frequency & 0xFF) | ((value & 0x07) << 8);

			if(value & 0x80) {
				//Start playback
				if(_state.Enabled && _state.Timer <= 2) {
					//Triggering the wave channel on DMG while the channel is reading a sample will corrupt the wave ram
					TriggerWaveRamCorruption();
				}

				//Channel is enabled, if DAC is enabled
				_state.Enabled = _state.DacEnabled;
				
				if(_state.Enabled) {
					UpdateOutput();
				}

				//Frequency timer is reloaded with period.
				_state.Timer = (2048 - _state.Frequency) * 2 + 6;

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
	if(!_state.Enabled) {
		_state.Ram[addr & 0x0F] = value;
	} else if(_allowRamAccess || _gameboy->IsCgb()) {
		//"On monochrome consoles, wave RAM can only be accessed on the same cycle that CH3 does. Otherwise, reads return $FF, and writes are ignored."
		//On CGB, the wave RAM can be accessed at any time, but will read/write the value the channel is currently accessing
		_state.Ram[_state.Position >> 1] = value;
	}
}

uint8_t GbWaveChannel::ReadRam(uint16_t addr)
{
	if(!_state.Enabled) {
		return _state.Ram[addr & 0x0F];
	} else if(_allowRamAccess || _gameboy->IsCgb()) {
		//"On monochrome consoles, wave RAM can only be accessed on the same cycle that CH3 does. Otherwise, reads return $FF, and writes are ignored."
		//On CGB, the wave RAM can be accessed at any time, but will read/write the value the channel is currently accessing
		return _state.Ram[_state.Position >> 1];
	} else {
		return 0xFF;
	}
}

void GbWaveChannel::TriggerWaveRamCorruption()
{
	if(_gameboy->IsCgb()) {
		return;
	}

	uint8_t pos = ((_state.Position + 1) & 0x1F) >> 1;
	if(pos < 4) {
		//"If the channel was reading one of the first four bytes, the only first byte will be rewritten with the byte being read."
		_state.Ram[0] = _state.Ram[pos];
	} else {
		//Otherwise, "the first FOUR bytes of wave RAM will be rewritten with the four aligned bytes that the read was from"
		memcpy(_state.Ram, _state.Ram + (pos & 0x0C), 4);
	}
}

void GbWaveChannel::Serialize(Serializer& s)
{
	SV(_state.DacEnabled); SV(_state.SampleBuffer); SV(_state.Position); SV(_state.Volume); SV(_state.Frequency);
	SV(_state.Length); SV(_state.LengthEnabled); SV(_state.Enabled); SV(_state.Timer); SV(_state.Output);
	SVArray(_state.Ram, 0x10); SV(_allowRamAccess);
	SV(_dac);
}
