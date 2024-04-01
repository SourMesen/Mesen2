#include "pch.h"
#include "Gameboy/APU/GbApu.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbTimer.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Audio/SoundMixer.h"
#include "Utilities/Serializer.h"

GbApu::GbApu()
{
	_soundBuffer = new int16_t[GbApu::MaxSamples * 2];
	memset(_soundBuffer, 0, GbApu::MaxSamples * 2 * sizeof(int16_t));

	_leftChannel = blip_new(GbApu::MaxSamples);
	_rightChannel = blip_new(GbApu::MaxSamples);
}

void GbApu::Init(Emulator* emu, Gameboy* gameboy)
{
	_square1.reset(new GbSquareChannel(this));
	_square2.reset(new GbSquareChannel(this));
	_wave.reset(new GbWaveChannel(this, gameboy));
	_noise.reset(new GbNoiseChannel(this));

	_prevLeftOutput = 0;
	_prevRightOutput = 0;
	_clockCounter = 0;
	_prevClockCount = 0;

	_emu = emu;
	_settings = emu->GetSettings();
	_soundMixer = emu->GetSoundMixer();
	_gameboy = gameboy;
	_state = {};

	blip_clear(_leftChannel);
	blip_clear(_rightChannel);

	blip_set_rates(_leftChannel, GbApu::ApuFrequency, GbApu::SampleRate);
	blip_set_rates(_rightChannel, GbApu::ApuFrequency, GbApu::SampleRate);
}

GbApu::~GbApu()
{
	blip_delete(_leftChannel);
	blip_delete(_rightChannel);
	delete[] _soundBuffer;
}

GbApuDebugState GbApu::GetState()
{
	GbApuDebugState state;
	state.Common = _state;
	state.Square1 = _square1->GetState();
	state.Square2 = _square2->GetState();
	state.Wave = _wave->GetState();
	state.Noise = _noise->GetState();
	return state;
}

bool GbApu::IsOddApuCycle()
{
	return ((_gameboy->GetApuCycleCount() - _powerOnCycle) & 0x02) != 0;
}

uint64_t GbApu::GetElapsedApuCycles()
{
	return _gameboy->GetApuCycleCount() - _powerOnCycle;
}

void GbApu::Run()
{
	uint64_t clockCount = _gameboy->GetApuCycleCount();
	uint32_t clocksToRun = (uint32_t)(clockCount - _prevClockCount);
	_prevClockCount = clockCount;

	GameboyConfig& cfg = _settings->GetGameboyConfig();

	if(!_state.ApuEnabled) {
		_clockCounter += clocksToRun;
	} else {
		while(clocksToRun > 0) {
			uint32_t minTimer = 250;
			if(clocksToRun < minTimer) {
				minTimer = clocksToRun;
			}
			if(_square1->GetState().Timer < minTimer) {
				minTimer = _square1->GetState().Timer;
			}
			if(_square2->GetState().Timer < minTimer) {
				minTimer = _square2->GetState().Timer;
			}
			if(_wave->GetState().Timer < minTimer) {
				minTimer = _wave->GetState().Timer;
			}
			if(_noise->GetState().Timer < minTimer) {
				minTimer = _noise->GetState().Timer;
			}

			clocksToRun -= minTimer;
			_square1->Exec(minTimer);
			_square2->Exec(minTimer);
			_wave->Exec(minTimer);
			_noise->Exec(minTimer);

			_clockCounter += minTimer;

			int16_t leftOutput = (
				(_square1->GetOutput() * (int32_t)(cfg.Square1Vol & _state.EnableLeftSq1) / 100) +
				(_square2->GetOutput() * (int32_t)(cfg.Square2Vol & _state.EnableLeftSq2) / 100) +
				(_wave->GetOutput() * (int32_t)(cfg.WaveVol & _state.EnableLeftWave) / 100) +
				(_noise->GetOutput() * (int32_t)(cfg.NoiseVol & _state.EnableLeftNoise) / 100)
			) * (_state.LeftVolume + 1) * 40;

			if(_prevLeftOutput != leftOutput) {
				blip_add_delta(_leftChannel, _clockCounter, leftOutput - _prevLeftOutput);
				_prevLeftOutput = leftOutput;
			}

			int16_t rightOutput = (
				(_square1->GetOutput() * (int32_t)(cfg.Square1Vol & _state.EnableRightSq1) / 100) +
				(_square2->GetOutput() * (int32_t)(cfg.Square2Vol & _state.EnableRightSq2) / 100) +
				(_wave->GetOutput() * (int32_t)(cfg.WaveVol & _state.EnableRightWave) / 100) +
				(_noise->GetOutput() * (int32_t)(cfg.NoiseVol & _state.EnableRightNoise) / 100)
			) * (_state.RightVolume + 1) * 40;

			if(_prevRightOutput != rightOutput) {
				blip_add_delta(_rightChannel, _clockCounter, rightOutput - _prevRightOutput);
				_prevRightOutput = rightOutput;
			}
		}
	}

	if(!_gameboy->IsSgb() && _clockCounter >= 20000) {
		PlayQueuedAudio();
	}
}

void GbApu::PlayQueuedAudio()
{
	blip_end_frame(_leftChannel, _clockCounter);
	blip_end_frame(_rightChannel, _clockCounter);

	uint32_t sampleCount = (uint32_t)blip_read_samples(_leftChannel, _soundBuffer, GbApu::MaxSamples, 1);
	blip_read_samples(_rightChannel, _soundBuffer + 1, GbApu::MaxSamples, 1);
	_soundMixer->PlayAudioBuffer(_soundBuffer, sampleCount, GbApu::SampleRate);
	_clockCounter = 0;
}

void GbApu::GetSoundSamples(int16_t* &samples, uint32_t& sampleCount)
{
	Run();
	blip_end_frame(_leftChannel, _clockCounter);
	blip_end_frame(_rightChannel, _clockCounter);

	sampleCount = (uint32_t)blip_read_samples(_leftChannel, _soundBuffer, GbApu::MaxSamples, 1);
	blip_read_samples(_rightChannel, _soundBuffer + 1, GbApu::MaxSamples, 1);
	samples = _soundBuffer;
	_clockCounter = 0;
}

void GbApu::ClockFrameSequencer()
{
	Run();

	if(!_state.ApuEnabled) {
		return;
	}

	if(_skipFirstEventCounter && --_skipFirstEventCounter) {
		return;
	}

	if((_state.FrameSequenceStep & 0x01) == 0) {
		_square1->ClockLengthCounter();
		_square2->ClockLengthCounter();
		_wave->ClockLengthCounter();
		_noise->ClockLengthCounter();

		if((_state.FrameSequenceStep & 0x03) == 2) {
			_square1->ClockSweepUnit();
		}
	} else if(_state.FrameSequenceStep == 7) {
		_square1->ClockEnvelope();
		_square2->ClockEnvelope();
		_noise->ClockEnvelope();
	}

	_state.FrameSequenceStep = (_state.FrameSequenceStep + 1) & 0x07;
}

uint8_t GbApu::Peek(uint16_t addr)
{
	return InternalRead(addr);
}

uint8_t GbApu::Read(uint16_t addr)
{
	Run();
	return InternalRead(addr);
}

uint8_t GbApu::InternalRead(uint16_t addr)
{
	switch(addr) {
		case 0xFF10: case 0xFF11: case 0xFF12: case 0xFF13: case 0xFF14:
			return _square1->Read(addr - 0xFF10);
			
		case 0xFF16: case 0xFF17: case 0xFF18: case 0xFF19:
			return _square2->Read(addr - 0xFF15);

		case 0xFF1A: case 0xFF1B: case 0xFF1C: case 0xFF1D: case 0xFF1E:
			return _wave->Read(addr - 0xFF1A);

		case 0xFF20: case 0xFF21: case 0xFF22: case 0xFF23:
			return _noise->Read(addr - 0xFF1F);

		case 0xFF24: 
			return (
				(_state.ExtAudioLeftEnabled ? 0x80 : 0) |
				(_state.LeftVolume << 4) |
				(_state.ExtAudioRightEnabled ? 0x08 : 0) |
				_state.RightVolume
			);

		case 0xFF25: 
			return (
				(_state.EnableLeftNoise ? 0x80 : 0) |
				(_state.EnableLeftWave ? 0x40 : 0) |
				(_state.EnableLeftSq2 ? 0x20 : 0) |
				(_state.EnableLeftSq1 ? 0x10 : 0) |
				(_state.EnableRightNoise ? 0x08 : 0) |
				(_state.EnableRightWave ? 0x04 : 0) |
				(_state.EnableRightSq2 ? 0x02 : 0) |
				(_state.EnableRightSq1 ? 0x01 : 0)
			);

		case 0xFF26:
			return (
				(_state.ApuEnabled ? 0x80 : 0) |
				0x70 | //open bus
				((_state.ApuEnabled && _noise->Enabled()) ? 0x08 : 0) |
				((_state.ApuEnabled && _wave->Enabled()) ? 0x04 : 0) |
				((_state.ApuEnabled && _square2->Enabled()) ? 0x02 : 0) |
				((_state.ApuEnabled && _square1->Enabled()) ? 0x01 : 0)
			);

		case 0xFF30: case 0xFF31: case 0xFF32: case 0xFF33: case 0xFF34: case 0xFF35: case 0xFF36: case 0xFF37:
		case 0xFF38: case 0xFF39: case 0xFF3A: case 0xFF3B: case 0xFF3C: case 0xFF3D: case 0xFF3E: case 0xFF3F:
			return _wave->ReadRam(addr);
	}

	//Open bus
	return 0xFF;
}

void GbApu::Write(uint16_t addr, uint8_t value)
{
	Run();

	if(!_state.ApuEnabled) {
		if(addr == 0xFF11 || addr == 0xFF16 || addr == 0xFF20) {
			//Allow writes to length counter, but not the upper 2 bits (square duty)
			value &= 0x3F;
		} else if(addr < 0xFF26 && addr != 0xFF1B) {
			//Ignore all writes to these registers when APU is disabled
			return;
		}
	}

	switch(addr) {
		case 0xFF10: case 0xFF11: case 0xFF12: case 0xFF13: case 0xFF14:
			_square1->Write(addr - 0xFF10, value);
			break;

		case 0xFF16: case 0xFF17: case 0xFF18: case 0xFF19:
			_square2->Write(addr - 0xFF15, value); //Same as square1, but without a sweep unit
			break;

		case 0xFF1A: case 0xFF1B: case 0xFF1C: case 0xFF1D: case 0xFF1E:
			_wave->Write(addr - 0xFF1A, value);
			break;

		case 0xFF20: case 0xFF21: case 0xFF22: case 0xFF23:
			_noise->Write(addr - 0xFF1F, value);
			break;

		case 0xFF24: 
			_state.ExtAudioLeftEnabled = (value & 0x80) != 0;
			_state.LeftVolume = (value & 0x70) >> 4;
			_state.ExtAudioRightEnabled = (value & 0x08) != 0;
			_state.RightVolume = (value & 0x07);
			break;

		case 0xFF25:
			_state.EnableLeftNoise = (value & 0x80) ? 0xFF : 0;
			_state.EnableLeftWave = (value & 0x40) ? 0xFF : 0;
			_state.EnableLeftSq2 = (value & 0x20) ? 0xFF : 0;
			_state.EnableLeftSq1 = (value & 0x10) ? 0xFF : 0;

			_state.EnableRightNoise = (value & 0x08) ? 0xFF : 0;
			_state.EnableRightWave = (value & 0x04) ? 0xFF : 0;
			_state.EnableRightSq2 = (value & 0x02) ? 0xFF : 0;
			_state.EnableRightSq1 = (value & 0x01) ? 0xFF : 0;
			break;

		case 0xFF26: {
			bool apuEnabled = (value & 0x80) != 0;
			if(_state.ApuEnabled != apuEnabled) {
				if(!apuEnabled) {
					_square1->Disable();
					_square2->Disable();
					_wave->Disable();
					_noise->Disable();
					Write(0xFF24, 0);
					Write(0xFF25, 0);
				} else {
					//"starting the APU while bit 4 of the DIV register is set causes the APU to skip the first DIV-APU event"
					//Based on the results of the div_*_10 tests in SameSuite, when the first event is skipped, the 
					//"length enable" glitch will be trigerred until the event after the skipped one occurs.
					//This uses a counter to reproduce this behavior
					_skipFirstEventCounter = _gameboy->GetTimer()->IsFrameSequencerBitSet() ? 2 : 0;

					//"When powered on, the frame sequencer is reset so that the next step will be 0,
					//the square duty units are reset to the first step of the waveform, and the wave channel's sample buffer is reset to 0."
					_state.FrameSequenceStep = 0;

					_powerOnCycle = _gameboy->GetApuCycleCount();

					_square1->Disable();
					_square2->Disable();
					_noise->Disable();
					_wave->Disable();

					if(_gameboy->IsCgb()) {
						//Length counters are reset to 0 at power on
						_square1->ResetLengthCounter();
						_square2->ResetLengthCounter();
						_wave->ResetLengthCounter();
						_noise->ResetLengthCounter();
					}
				}
				_state.ApuEnabled = apuEnabled;
			}
			break;
		}
		case 0xFF30: case 0xFF31: case 0xFF32: case 0xFF33: case 0xFF34: case 0xFF35: case 0xFF36: case 0xFF37:
		case 0xFF38: case 0xFF39: case 0xFF3A: case 0xFF3B: case 0xFF3C: case 0xFF3D: case 0xFF3E: case 0xFF3F:
			_wave->WriteRam(addr, value);
			break;
	}
}

uint8_t GbApu::ReadCgbRegister(uint16_t addr)
{
	Run();

	switch(addr) {
		case 0xFF76: return _square1->GetRawOutput() | (_square2->GetRawOutput() << 4);
		case 0xFF77: return _wave->GetRawOutput() | (_noise->GetRawOutput() << 4);
	}

	//Should not be called
	return 0;
}

template<typename T>
void GbApu::ProcessLengthEnableFlag(uint8_t value, T &length, bool &lengthEnabled, bool &enabled)
{
	bool newLengthEnabled = (value & 0x40) != 0;
	if(newLengthEnabled && !lengthEnabled && ((_state.FrameSequenceStep & 0x01) == 1 || _skipFirstEventCounter)) {
		//"Extra length clocking occurs when writing to NRx4 when the frame sequencer's next step is one that doesn't clock
		//the length counter. In this case, if the length counter was PREVIOUSLY disabled and now enabled and the length counter
		//is not zero, it is decremented. If this decrement makes it zero and trigger is clear, the channel is disabled."
		if(length > 0) {
			length--;
			if(length == 0) {
				if(value & 0x80) {
					length = sizeof(T) == 1 ? 0x3F : 0xFF;
				} else {
					enabled = false;
				}
			}
		}
	}
	lengthEnabled = newLengthEnabled;
}

void GbApu::Serialize(Serializer& s)
{
	if(s.IsSaving()) {
		Run();
	} else {
		_clockCounter = 0;
		blip_clear(_leftChannel);
		blip_clear(_rightChannel);
	}

	SV(_state.ApuEnabled); SV(_state.FrameSequenceStep);
	SV(_state.EnableLeftSq1); SV(_state.EnableLeftSq2); SV(_state.EnableLeftWave); SV(_state.EnableLeftNoise);
	SV(_state.EnableRightSq1); SV(_state.EnableRightSq2); SV(_state.EnableRightWave); SV(_state.EnableRightNoise);
	SV(_state.LeftVolume); SV(_state.RightVolume); SV(_state.ExtAudioLeftEnabled); SV(_state.ExtAudioRightEnabled);
	SV(_prevLeftOutput); SV(_prevRightOutput); SV(_clockCounter); SV(_prevClockCount); SV(_skipFirstEventCounter);
	SV(_powerOnCycle);

	SV(_square1);
	SV(_square2);
	SV(_wave);
	SV(_noise);
}

template void GbApu::ProcessLengthEnableFlag<uint8_t>(uint8_t value, uint8_t& length, bool& lengthEnabled, bool& enabled);
template void GbApu::ProcessLengthEnableFlag<uint16_t>(uint8_t value, uint16_t& length, bool& lengthEnabled, bool& enabled);