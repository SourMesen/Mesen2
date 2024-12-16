#include "pch.h"
#include "PCE/PcePsg.h"
#include "PCE/PceConsole.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/MessageManager.h"
#include "Shared/Audio/SoundMixer.h"
#include "Utilities/Audio/blip_buf.h"
#include "Utilities/Serializer.h"

PcePsg::PcePsg(Emulator* emu, PceConsole* console)
{
	_emu = emu;
	_console = console;
	_soundMixer = emu->GetSoundMixer();

	_soundBuffer = new int16_t[PcePsg::MaxSamples * 2];
	memset(_soundBuffer, 0, PcePsg::MaxSamples * 2 * sizeof(int16_t));

	for(int i = 0; i < 6; i++) {
		_channels[i].Init(i, this);
	}

	UpdateSoundOffset();

	_leftChannel = blip_new(PcePsg::MaxSamples);
	_rightChannel = blip_new(PcePsg::MaxSamples);

	blip_clear(_leftChannel);
	blip_clear(_rightChannel);

	blip_set_rates(_leftChannel, PcePsg::PsgFrequency, PcePsg::SampleRate);
	blip_set_rates(_rightChannel, PcePsg::PsgFrequency, PcePsg::SampleRate);
}

PcePsg::~PcePsg()
{
	blip_delete(_leftChannel);
	blip_delete(_rightChannel);
	delete[] _soundBuffer;
}

bool PcePsg::IsLfoEnabled()
{
	return (_state.LfoControl & 0x80) == 0 && (_state.LfoControl & 0x03);
}

uint16_t PcePsg::GetLfoFrequency()
{
	return _state.LfoFrequency ? _state.LfoFrequency : 0x100;
}

uint32_t PcePsg::GetLfoCh1PeriodOffset()
{
	//When LFO is enabled, the period of channel 1 is altered
	//based on channel's 2 current output, multiplied by either 1, 4 or 16
	//depending on the value in the lower 2 bits of the LFO's control register
	int shift = ((_state.LfoControl & 0x03) - 1) * 2;
	int8_t ch2Out = _channels[1].GetState().CurrentOutput;
	return (uint32_t)(ch2Out << shift);
}

void PcePsg::Write(uint16_t addr, uint8_t value)
{
	Run();

	switch(addr & 0x0F) {
		case 0: _state.ChannelSelect = value & 0x07; break;
		case 1:
			_state.RightVolume = value & 0x0F;
			_state.LeftVolume = (value >> 4) & 0x0F;
			break;

		case 2: case 3: case 4: case 5: case 6:
		{
			if(_state.ChannelSelect < 6) {
				_channels[_state.ChannelSelect].Write(addr, value);
			}
			break;
		}

		case 7:
			//Only channels 5/6 have noise
			if(_state.ChannelSelect == 4 || _state.ChannelSelect == 5) {
				_channels[_state.ChannelSelect].Write(addr, value);
			}
			break;

		case 8: _state.LfoFrequency = value; break;
		case 9: _state.LfoControl = value; break;
	}

	UpdateOutput(_emu->GetSettings()->GetPcEngineConfig());
}

void PcePsg::Run()
{
	uint64_t clock = _console->GetMasterClock();
	uint32_t clocksToRun = clock - _lastClock;
	PcEngineConfig& cfg = _emu->GetSettings()->GetPcEngineConfig();
	while(clocksToRun >= 6) {
		uint32_t minTimer = clocksToRun / 6;
		for(int i = 0; i < 6; i++) {
			uint16_t timer = _channels[i].GetTimer();
			if(timer != 0 && timer < minTimer) {
				minTimer = timer;
			}
		}

		for(int i = 0; i < 6; i++) {
			_channels[i].Run(minTimer);
		}

		_clockCounter += minTimer;
		clocksToRun -= minTimer * 6;

		UpdateOutput(cfg);
	}

	if(_clockCounter >= 20000) {
		PlayQueuedAudio();
	}

	_lastClock = clock - clocksToRun;
}

void PcePsg::UpdateOutput(PcEngineConfig& cfg)
{
	int16_t leftOutput = 0;
	int16_t rightOutput = 0;
	for(int i = 0; i < 6; i++) {
		PcePsgChannel& ch = _channels[i];
		leftOutput += (int32_t)ch.GetOutput(true, _state.LeftVolume) * (int32_t)cfg.ChannelVol[i] / 100;
		rightOutput += (int32_t)ch.GetOutput(false, _state.RightVolume) * (int32_t)cfg.ChannelVol[i] / 100;
	}

	if(_prevLeftOutput != leftOutput) {
		blip_add_delta(_leftChannel, _clockCounter, leftOutput - _prevLeftOutput);
		_prevLeftOutput = leftOutput;
	}

	if(_prevRightOutput != rightOutput) {
		blip_add_delta(_rightChannel, _clockCounter, rightOutput - _prevRightOutput);
		_prevRightOutput = rightOutput;
	}
}

void PcePsg::UpdateSoundOffset()
{
	uint8_t offset = _emu->GetSettings()->GetPcEngineConfig().UseHuC6280aAudio ? 0x10 : 0;
	for(int i = 0; i < 6; i++) {
		_channels[i].SetOutputOffset(offset);
	}
}

void PcePsg::PlayQueuedAudio()
{
	blip_end_frame(_leftChannel, _clockCounter);
	blip_end_frame(_rightChannel, _clockCounter);

	uint32_t sampleCount = (uint32_t)blip_read_samples(_leftChannel, _soundBuffer, PcePsg::MaxSamples, 1);
	blip_read_samples(_rightChannel, _soundBuffer + 1, PcePsg::MaxSamples, 1);
	_soundMixer->PlayAudioBuffer(_soundBuffer, sampleCount, PcePsg::SampleRate);
	_clockCounter = 0;
	
	UpdateSoundOffset();
}

void PcePsg::Serialize(Serializer& s)
{
	SV(_state.ChannelSelect);
	SV(_state.LeftVolume);
	SV(_state.LfoControl);
	SV(_state.LfoFrequency);
	SV(_state.RightVolume);

	if(s.GetFormat() != SerializeFormat::Map) {
		//Hide these entries from the Lua API
		SV(_lastClock);
		SV(_prevLeftOutput);
		SV(_prevRightOutput);
		SV(_clockCounter);
	}

	for(int i = 0; i < 6; i++) {
		SVI(_channels[i]);
	}
}
