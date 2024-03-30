#include "pch.h"
#include "GBA/APU/GbaWaveChannel.h"
#include "GBA/APU/GbaApu.h"
#include "GBA/GbaConsole.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"

GbaWaveChannel::GbaWaveChannel(GbaApu* apu, GbaConsole* console)
{
	_console = console;
	_apu = apu;

	_console->InitializeRam(_state.Ram, 0x20);
}

GbaWaveState& GbaWaveChannel::GetState()
{
	return _state;
}

bool GbaWaveChannel::Enabled()
{
	return _state.Enabled;
}

void GbaWaveChannel::Disable()
{
	uint16_t len = _state.Length;
	uint8_t ram[0x10];
	memcpy(ram, _state.Ram, sizeof(ram));
	_state = {};
	_state.Length = len;
	memcpy(_state.Ram, ram, sizeof(ram));
	_state.Timer = 2048 * 2;
}

void GbaWaveChannel::ResetLengthCounter()
{
	_state.Length = 0;
}

void GbaWaveChannel::ClockLengthCounter()
{
	if(_state.LengthEnabled && _state.Length > 0) {
		_state.Length--;
		if(_state.Length == 0) {
			//"Length becoming 0 should clear status"
			_state.Enabled = false;
			_apu->UpdateEnabledChannels();
			_state.Output = 0;
		}
	}
}

uint8_t GbaWaveChannel::GetRawOutput()
{
	//"Stopping channel 3 manually using the NR30 register affects PCM34 instantly" (SameSuite's channel_3_stop_delay test)
	return _state.Enabled ? _state.Output : 0;
}

double GbaWaveChannel::GetOutput()
{
	return _state.Output;
}

void GbaWaveChannel::UpdateOutput()
{
	if(_state.OverrideVolume) {
		_state.Output = _state.SampleBuffer * 3 / 4;
	} else if(_state.Volume) {
		_state.Output = _state.SampleBuffer >> (_state.Volume - 1);
	} else {
		_state.Output = 0;
	}
}

void GbaWaveChannel::Exec(uint32_t clocksToRun)
{
	if(!_state.Enabled) {
		return;
	}
	
	bool needUpdate = false;
	do {
		uint32_t minTimer = std::min<uint32_t>(clocksToRun, _state.Timer);
		_state.Timer -= minTimer;
		_allowRamAccess = false;

		if(_state.Timer == 0) {
			//The wave channel's frequency timer period is set to (2048-frequency)*2.
			_state.Timer = (2048 - _state.Frequency) * 2;

			//When the timer generates a clock, the position counter is advanced one sample in the wave table,
			//looping back to the beginning when it goes past the end,
			_state.Position = (_state.Position + 1) & (_state.DoubleLength ? 0x3F : 0x1F);
			uint8_t bank = _state.DoubleLength ? 0 : (_state.SelectedBank << 4);

			//then a sample is read into the sample buffer from this NEW position.
			if(_state.Position & 0x01) {
				_state.SampleBuffer = _state.Ram[bank | (_state.Position >> 1)] & 0x0F;
			} else {
				_state.SampleBuffer = _state.Ram[bank | (_state.Position >> 1)] >> 4;
			}

			//The DAC receives the current value from the upper/lower nibble of the sample buffer, shifted right by the volume control. 
			needUpdate = true;

			_allowRamAccess = true;
		}
		clocksToRun -= minTimer;
	} while(clocksToRun);

	if(needUpdate) {
		UpdateOutput();
	}
}

uint8_t GbaWaveChannel::Read(uint16_t addr)
{
	switch(addr) {
		case 0: return (
			(_state.DacEnabled ? 0x80 : 0) |
			(_state.SelectedBank ? 0x40 : 0) |
			(_state.DoubleLength ? 0x20 : 0)
		);

		case 2: return (
			(_state.OverrideVolume ? 0x80 : 0) |
			(_state.Volume << 5)
		);

		case 4: return _state.LengthEnabled ? 0x40 : 0;
	}

	return 0;
}

void GbaWaveChannel::Write(uint16_t addr, uint8_t value)
{
	switch(addr) {
		case 0:
			_state.DacEnabled = value & 0x80;
			_state.SelectedBank = (value & 0x40) >> 6;
			_state.DoubleLength = value & 0x20;
			_state.Enabled &= _state.DacEnabled;
			_apu->UpdateEnabledChannels();
			break;

		case 1:
			_state.Length = 256 - value;
			break;

		case 2:
			_state.Volume = (value & 0x60) >> 5;
			_state.OverrideVolume = value & 0x80;

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
				//Channel is enabled, if DAC is enabled
				_state.Enabled = _state.DacEnabled;
				_apu->UpdateEnabledChannels();
				
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

			_state.LengthEnabled = (value & 0x40);
			break;
		}
	}
}

void GbaWaveChannel::WriteRam(uint16_t addr, uint8_t value)
{
	uint8_t bank = (_state.SelectedBank ^ 1) << 4;
	if(!_state.Enabled) {
		_state.Ram[bank | (addr & 0x0F)] = value;
	} else if(_allowRamAccess) {
		//TODOGBA when is wave ram accessible on gba?
		_state.Ram[_state.Position >> 1] = value;
	}
}

uint8_t GbaWaveChannel::ReadRam(uint16_t addr)
{
	uint8_t bank = (_state.SelectedBank ^ 1) << 4;
	if(!_state.Enabled) {
		return _state.Ram[bank | (addr & 0x0F)];
	} else if(_allowRamAccess) {
		//TODOGBA when is wave ram accessible on gba?
		return _state.Ram[_state.Position >> 1];
	} else {
		return 0xFF;
	}
}

void GbaWaveChannel::Serialize(Serializer& s)
{
	SV(_state.DacEnabled); SV(_state.SampleBuffer); SV(_state.Position); SV(_state.Volume); SV(_state.Frequency);
	SV(_state.Length); SV(_state.LengthEnabled); SV(_state.Enabled); SV(_state.Timer); SV(_state.Output);
	SVArray(_state.Ram, 0x20); SV(_allowRamAccess);
	SV(_state.DoubleLength);
	SV(_state.SelectedBank);
	SV(_state.OverrideVolume);
}
