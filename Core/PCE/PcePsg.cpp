#include "stdafx.h"
#include "PCE/PcePsg.h"
#include "Shared/Emulator.h"
#include "Shared/MessageManager.h"
#include "Shared/Audio/SoundMixer.h"
#include "Utilities/Audio/blip_buf.h"

PcePsg::PcePsg(Emulator* emu)
{
	_emu = emu;
	_soundMixer = emu->GetSoundMixer();

	_soundBuffer = new int16_t[PcePsg::MaxSamples * 2];
	memset(_soundBuffer, 0, PcePsg::MaxSamples * 2 * sizeof(int16_t));

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

		//TODO, LFO is not implemented
		case 8: _state.LfoFrequency = value; break;
		case 9: _state.LfoControl = value; break;
	}
}

void PcePsg::Run()
{
	uint64_t clock = _emu->GetMasterClock();
	uint32_t clocksToRun = clock - _lastClock;
	while(clocksToRun >= 6) {
		uint32_t minTimer = clocksToRun / 6;
		for(int i = 0; i < 6; i++) {
			uint16_t timer = _channels[i].GetTimer();
			if(timer != 0 && timer < minTimer) {
				minTimer = timer;
			}
		}

		int16_t leftOutput = 0;
		int16_t rightOutput = 0;
		for(int i = 0; i < 6; i++) {
			PcePsgChannel& ch = _channels[i];
			ch.Run(minTimer);
			leftOutput += ch.GetOutput(true, _state.LeftVolume);
			rightOutput += ch.GetOutput(false, _state.RightVolume);
		}

		if(_prevLeftOutput != leftOutput) {
			blip_add_delta(_leftChannel, _clockCounter, leftOutput - _prevLeftOutput);
			_prevLeftOutput = leftOutput;
		}

		if(_prevRightOutput != rightOutput) {
			blip_add_delta(_rightChannel, _clockCounter, rightOutput - _prevRightOutput);
			_prevRightOutput = rightOutput;
		}

		_clockCounter += minTimer;
		clocksToRun -= minTimer * 6;
	}

	if(_clockCounter >= 20000) {
		blip_end_frame(_leftChannel, _clockCounter);
		blip_end_frame(_rightChannel, _clockCounter);

		uint32_t sampleCount = (uint32_t)blip_read_samples(_leftChannel, _soundBuffer, PcePsg::MaxSamples, 1);
		blip_read_samples(_rightChannel, _soundBuffer + 1, PcePsg::MaxSamples, 1);
		_soundMixer->PlayAudioBuffer(_soundBuffer, sampleCount, PcePsg::SampleRate);
		_clockCounter = 0;
	}

	_lastClock = clock - clocksToRun;
}
