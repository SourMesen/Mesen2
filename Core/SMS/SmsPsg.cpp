#include "pch.h"
#include "SMS/SmsPsg.h"
#include "SMS/SmsFmAudio.h"
#include "Utilities/Serializer.h"

SmsPsg::SmsPsg(Emulator* emu, SmsConsole* console)
{
	_console = console;
	_soundMixer = emu->GetSoundMixer();
	_settings = emu->GetSettings();

	_state.Noise.Lfsr = 0x8000;
	_state.Noise.Volume = 0x0F;
	_state.Tone[0].Volume = 0x0F;
	_state.Tone[1].Volume = 0x0F;
	_state.Tone[2].Volume = 0x0F;

	_soundBuffer = new int16_t[SmsPsg::MaxSamples * 2];
	memset(_soundBuffer, 0, SmsPsg::MaxSamples * 2 * sizeof(int16_t));

	_leftChannel = blip_new(SmsPsg::MaxSamples);
	_rightChannel = blip_new(SmsPsg::MaxSamples);

	blip_clear(_leftChannel);
	blip_clear(_rightChannel);

	blip_set_rates(_leftChannel, _console->GetMasterClockRate(), SmsPsg::SampleRate);
	blip_set_rates(_rightChannel, _console->GetMasterClockRate(), SmsPsg::SampleRate);
}

void SmsPsg::SetRegion(ConsoleRegion region)
{
	blip_clear(_leftChannel);
	blip_clear(_rightChannel);

	blip_set_rates(_leftChannel, _console->GetMasterClockRate(), SmsPsg::SampleRate);
	blip_set_rates(_rightChannel, _console->GetMasterClockRate(), SmsPsg::SampleRate);
}

void SmsPsg::RunNoise(SmsNoiseChannelState& noise)
{
	if(noise.Timer == 0 || --noise.Timer == 0) {
		noise.LfsrInputBit ^= 1;
		switch(noise.Control & 0x03) {
			case 0: noise.Timer = 0x10; break;
			case 1: noise.Timer = 0x20; break;
			case 2: noise.Timer = 0x40; break;
			case 3: noise.Timer = _state.Tone[2].ReloadValue; break;
		}

		if(noise.LfsrInputBit) {
			bool useBit3 = noise.Control & 0x04;
			uint16_t newBit = (noise.Lfsr & 0x01) ^ (useBit3 & ((noise.Lfsr >> 3) & 0x01));
			noise.Lfsr = (newBit << 15) | (noise.Lfsr >> 1);
			noise.Output = noise.Lfsr & 0x01;
		}
	}
}

void SmsPsg::Run()
{
	uint64_t runTo = _console->GetMasterClock();
	uint32_t* volumes = _console->GetModel() == SmsModel::ColecoVision ? _settings->GetCvConfig().ChannelVolumes : _settings->GetSmsConfig().ChannelVolumes;

	while(_masterClock + 16 < runTo) {
		int16_t outputLeft = 0;
		int16_t outputRight = 0;
		int16_t channelOutput;
		for(int i = 0; i < 3; i++) {
			if(_state.Tone[i].Timer == 0 || --_state.Tone[i].Timer == 0) {
				_state.Tone[i].Output ^= 1;
				_state.Tone[i].Timer = _state.Tone[i].ReloadValue;
			}

			channelOutput = _state.Tone[i].Output * _volumeLut[_state.Tone[i].Volume] * volumes[i] / 100;
			if(_state.GameGearPanningReg & (0x01 << i)) {
				outputRight += channelOutput;
			}
			if(_state.GameGearPanningReg & (0x10 << i)) {
				outputLeft += channelOutput;
			}
		}

		RunNoise(_state.Noise);
		channelOutput = _state.Noise.Output * _volumeLut[_state.Noise.Volume] * volumes[3] / 100;
		if(_state.GameGearPanningReg & 0x08) {
			outputRight += channelOutput;
		}
		if(_state.GameGearPanningReg & 0x80) {
			outputLeft += channelOutput;
		}

		_clockCounter += 16;
		_masterClock += 16;

		if(_prevOutputLeft != outputLeft || _prevOutputRight != outputRight) {
			blip_add_delta(_leftChannel, _clockCounter, outputLeft - _prevOutputLeft);
			blip_add_delta(_rightChannel, _clockCounter, outputRight - _prevOutputRight);
			_prevOutputLeft = outputLeft;
			_prevOutputRight = outputRight;
		}
	}

	if(_clockCounter >= 20000) {
		PlayQueuedAudio();
	}
}

void SmsPsg::PlayQueuedAudio()
{
	blip_end_frame(_leftChannel, _clockCounter);
	blip_end_frame(_rightChannel, _clockCounter);

	uint32_t sampleCount = (uint32_t)blip_read_samples(_leftChannel, _soundBuffer, SmsPsg::MaxSamples, 1);
	blip_read_samples(_rightChannel, _soundBuffer + 1, SmsPsg::MaxSamples, 1);

	if(_console->IsPsgAudioMuted()) {
		memset(_soundBuffer, 0, SmsPsg::MaxSamples * 2 * sizeof(int16_t));
	}

	_soundMixer->PlayAudioBuffer(_soundBuffer, sampleCount, SmsPsg::SampleRate);
	_clockCounter = 0;
}

void SmsPsg::Write(uint8_t value)
{
	Run();

	if(value & 0x80) {
		_state.SelectedReg = (value >> 4) & 0x07;
	}

	uint8_t channel = (_state.SelectedReg >> 1) & 0x03;
	bool volReg = _state.SelectedReg & 0x01;

	switch(channel) {
		case 0: case 1: case 2:
			if(volReg) {
				_state.Tone[channel].Volume = value & 0x0F;
			} else {
				if(value & 0x80) {
					_state.Tone[channel].ReloadValue = (_state.Tone[channel].ReloadValue & 0x3F0) | (value & 0x0F);
				} else {
					_state.Tone[channel].ReloadValue = (_state.Tone[channel].ReloadValue & 0x0F) | ((value & 0x3F) << 4);
				}
			}
			break;

		case 3:
			if(volReg) {
				_state.Noise.Volume = value & 0x0F;
			} else {
				_state.Noise.Control = value & 0x07;
				_state.Noise.Lfsr = 0x8000;
			}
			break;
	}
}

void SmsPsg::WritePanningReg(uint8_t value)
{
	_state.GameGearPanningReg = value;
}

void SmsPsg::Serialize(Serializer& s)
{
	if(s.IsSaving()) {
		Run();
	} else {
		_clockCounter = 0;
		blip_clear(_leftChannel);
		blip_clear(_rightChannel);
	}

	SV(_state.SelectedReg);
	SV(_state.GameGearPanningReg);

	SV(_state.Noise.Timer);
	SV(_state.Noise.Lfsr);
	SV(_state.Noise.LfsrInputBit);
	SV(_state.Noise.Control);
	SV(_state.Noise.Output);
	SV(_state.Noise.Volume);

	for(int i = 0; i < 3; i++) {
		SVI(_state.Tone[i].ReloadValue);
		SVI(_state.Tone[i].Timer);
		SVI(_state.Tone[i].Output);
		SVI(_state.Tone[i].Volume);
	}

	if(s.GetFormat() != SerializeFormat::Map) {
		//Hide these entries from the Lua API
		SV(_masterClock);
		SV(_clockCounter);
		SV(_prevOutputLeft);
		SV(_prevOutputRight);
	}
}
