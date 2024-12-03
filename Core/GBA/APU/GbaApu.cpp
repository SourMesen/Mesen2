#include "pch.h"
#include "GBA/APU/GbaApu.h"
#include "GBA/APU/GbaSquareChannel.h"
#include "GBA/APU/GbaNoiseChannel.h"
#include "GBA/APU/GbaWaveChannel.h"
#include "GBA/GbaConsole.h"
#include "GBA/GbaDmaController.h"
#include "GBA/GbaMemoryManager.h"
#include "Shared/MessageManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Audio/SoundMixer.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/BitUtilities.h"
#include "Utilities/Serializer.h"
#include "Utilities/StaticFor.h"

//TODOGBA APU todo list
//-do channels output 0 to 15, or -7 to 7? If -7 to 7, what does "0" correspond to?
//-GBA vs GB zombie mode (currently identical to the GB core)
//-a lot of gbc/gb behavior is implemented as-is (same as the GB core) - unsure how much of it is accurate or incorrect for GBA
//-does the length counter extra clock glitch exist on gba?
//-when is wave ram access possible?

void GbaApu::Init(Emulator* emu, GbaConsole* console, GbaDmaController* dmaController, GbaMemoryManager* memoryManager)
{
	_soundBuffer = new int16_t[GbaApu::MaxSamples];
	memset(_soundBuffer, 0, GbaApu::MaxSamples * sizeof(int16_t));

	_dmaController = dmaController;
	_memoryManager = memoryManager;

	_square1.reset(new GbaSquareChannel(this));
	_square2.reset(new GbaSquareChannel(this));
	_wave.reset(new GbaWaveChannel(this, console));
	_noise.reset(new GbaNoiseChannel(this));

	_sampleCount = 0;
	_prevClockCount = 0;

	_state = {};
	_state.Bias = 0x200;

	UpdateSampleRate();

	_emu = emu;
	_console = console;
	_settings = emu->GetSettings();
	_soundMixer = emu->GetSoundMixer();

	StaticFor<0, 16>::Apply([=](auto i) {
		_runFunc[i] = &GbaApu::InternalRun<(bool)(i & 0x01), (bool)(i & 0x02), (bool)(i & 0x04), (bool)(i & 0x08)>;
	});
}

GbaApu::GbaApu()
{
}

GbaApu::~GbaApu()
{
	delete[] _soundBuffer;
}

template<bool sq1Enabled, bool sq2Enabled, bool waveEnabled, bool noiseEnabled>
void GbaApu::InternalRun()
{
	uint64_t clockCount = _console->GetMasterClock() / 4;
	if(clockCount == _prevClockCount) {
		return;
	}
	
	uint32_t clocksToRun = (uint32_t)(clockCount - _prevClockCount);

	GbaConfig& cfg = _settings->GetGbaConfig();

	uint64_t samplingRate = 0x08ll << (3 - _state.SamplingRate);
	int16_t bitRateMask = ~0;
	switch(_state.SamplingRate) {
		case 0: break;
		case 1: bitRateMask = ~1; break;
		case 2: bitRateMask = ~3; break;
		case 3: bitRateMask = ~7; break;
	}

	bool enabled = _state.ApuEnabled && !_memoryManager->IsSystemStopped();

	bool changed = true;
	while(clocksToRun > 0) {
		uint32_t minTimer = clocksToRun < samplingRate ? clocksToRun : samplingRate;

		if(enabled) {
			if constexpr(sq1Enabled) {
				uint8_t output = _square1->GetRawOutput();
				_square1->Exec(minTimer);
				changed |= output != _square1->GetRawOutput();
			}

			if constexpr(sq2Enabled) {
				uint8_t output = _square2->GetRawOutput();
				_square2->Exec(minTimer);
				changed |= output != _square2->GetRawOutput();
			}

			if constexpr(waveEnabled) {
				uint8_t output = _wave->GetRawOutput();
				_wave->Exec(minTimer);
				changed |= output != _wave->GetRawOutput();
			}

			if constexpr(noiseEnabled) {
				uint8_t output = _noise->GetRawOutput();
				_noise->Exec(minTimer);
				changed |= output != _noise->GetRawOutput();
			}

			if((_prevClockCount & 0x1000) && !(_prevClockCount + minTimer & 0x1000)) {
				ClockFrameSequencer();
			}
		}

		bool updateSample = (_prevClockCount & samplingRate) && !(_prevClockCount + minTimer & samplingRate);
		_prevClockCount += minTimer;
		clocksToRun -= minTimer;

		if(updateSample) {
			if(changed) {
				changed = false;

				double gbVolume = _state.GbVolume ? _state.GbVolume : 0.5;

				int16_t gbLeftOutput = (
					(_square1->GetOutput() * (int32_t)(cfg.Square1Vol & _state.EnableLeftSq1) / 100) +
					(_square2->GetOutput() * (int32_t)(cfg.Square2Vol & _state.EnableLeftSq2) / 100) +
					(_wave->GetOutput() * (int32_t)(cfg.WaveVol & _state.EnableLeftWave) / 100) +
					(_noise->GetOutput() * (int32_t)(cfg.NoiseVol & _state.EnableLeftNoise) / 100)
				) * (_state.LeftVolume + 1) * gbVolume;

				_leftSample = ((std::clamp(
					_state.Bias +
					gbLeftOutput +
					((_state.EnableLeftA ? (_state.DmaSampleA * (int32_t)cfg.ChannelAVol / 100) : 0) << (_state.VolumeA + 1)) +
					((_state.EnableLeftB ? (_state.DmaSampleB * (int32_t)cfg.ChannelBVol / 100) : 0) << (_state.VolumeB + 1))
				, 0, 0x3FF) & bitRateMask) - _state.Bias) * 32;

				int16_t gbRightOutput = (
					(_square1->GetOutput() * (int32_t)(cfg.Square1Vol & _state.EnableRightSq1) / 100) +
					(_square2->GetOutput() * (int32_t)(cfg.Square2Vol & _state.EnableRightSq2) / 100) +
					(_wave->GetOutput() * (int32_t)(cfg.WaveVol & _state.EnableRightWave) / 100) +
					(_noise->GetOutput() * (int32_t)(cfg.NoiseVol & _state.EnableRightNoise) / 100)
				) * (_state.RightVolume + 1) * gbVolume;

				_rightSample = ((std::clamp(
					_state.Bias +
					gbRightOutput +
					((_state.EnableRightA ? (_state.DmaSampleA * (int32_t)cfg.ChannelAVol / 100) : 0) << (_state.VolumeA + 1)) +
					((_state.EnableRightB ? (_state.DmaSampleB * (int32_t)cfg.ChannelBVol / 100) : 0) << (_state.VolumeB + 1))
				, 0, 0x3FF) & bitRateMask) - _state.Bias) * 32;
			}

			//Use low pass filter and subtract the result to filter out DC offset
			_soundBuffer[_sampleCount] = _leftSample - _filterL.Process(_leftSample);
			_soundBuffer[_sampleCount + 1] = _rightSample - _filterR.Process(_rightSample);
			_sampleCount += 2;
		}
	}

	if(_sampleCount >= 2000) {
		PlayQueuedAudio();
	}

	_prevClockCount = clockCount;
}

void GbaApu::PlayQueuedAudio()
{
	_soundMixer->PlayAudioBuffer(_soundBuffer, _sampleCount / 2, _sampleRate);
	_sampleCount = 0;
}

void GbaApu::ClockFrameSequencer()
{
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


uint8_t GbaApu::ReadRegister(uint32_t addr)
{
	switch(addr) {
		case 0x60: return _square1->Read(0);
		case 0x61: return 0;
		case 0x62: return _square1->Read(1);
		case 0x63: return _square1->Read(2);
		case 0x64: return _square1->Read(3);
		case 0x65: return _square1->Read(4);
		case 0x66: return 0;
		case 0x67: return 0;

		case 0x68: return _square2->Read(1);
		case 0x69: return _square2->Read(2);
		case 0x6A: return 0;
		case 0x6B: return 0;
		case 0x6C: return _square2->Read(3);
		case 0x6D: return _square2->Read(4);
		case 0x6E: return 0;
		case 0x6F: return 0;

		case 0x70: return _wave->Read(0);
		case 0x71: return 0;
		case 0x72: return _wave->Read(1);
		case 0x73: return _wave->Read(2);
		case 0x74: return _wave->Read(3);
		case 0x75: return _wave->Read(4);
		case 0x76: return 0;
		case 0x77: return 0;

		case 0x78: return _noise->Read(1);
		case 0x79: return _noise->Read(2);
		case 0x7A: return 0;
		case 0x7B: return 0;
		case 0x7C: return _noise->Read(3);
		case 0x7D: return _noise->Read(4);
		case 0x7E: return 0;
		case 0x7F: return 0;

		case 0x80: return _state.RightVolume | (_state.LeftVolume << 4);
		case 0x81: return _state.EnabledGb;

		case 0x82: return _state.VolumeControl;
		case 0x83: return _state.DmaSoundControl;

		case 0x84:
			return (
				(_state.ApuEnabled ? 0x80 : 0) |
				((_state.ApuEnabled && _noise->Enabled()) ? 0x08 : 0) |
				((_state.ApuEnabled && _wave->Enabled()) ? 0x04 : 0) |
				((_state.ApuEnabled && _square2->Enabled()) ? 0x02 : 0) |
				((_state.ApuEnabled && _square1->Enabled()) ? 0x01 : 0)
			);

		case 0x85: return 0;
		case 0x86: return 0;
		case 0x87: return 0;

		case 0x88: return (uint8_t)_state.Bias;
		case 0x89: return (uint8_t)(_state.Bias >> 8) | (_state.SamplingRate << 6);
		case 0x8A: return 0;
		case 0x8B: return 0;

		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E: case 0x9F:
			return _wave->ReadRam(addr);

		default:
			LogDebug("Read unknown sound register: " + HexUtilities::ToHex32(addr));
			return _memoryManager->GetOpenBus(addr);
	}
}

void GbaApu::WriteRegister(GbaAccessModeVal mode, uint32_t addr, uint8_t value)
{
	Run();

	if(!_state.ApuEnabled && addr <= 0x81) {
		//Ignore all writes to these registers when APU is disabled
		return;
	}

	switch(addr) {
		case 0x60: _square1->Write(0, value); break;
		case 0x61: break;
		case 0x62: _square1->Write(1, value); break;
		case 0x63: _square1->Write(2, value); break;
		case 0x64: _square1->Write(3, value); break;
		case 0x65: _square1->Write(4, value); break;
		case 0x66: break;
		case 0x67: break;

		case 0x68: _square2->Write(1, value); break;
		case 0x69: _square2->Write(2, value); break;
		case 0x6A: break;
		case 0x6B: break;
		case 0x6C: _square2->Write(3, value); break;
		case 0x6D: _square2->Write(4, value); break;
		case 0x6E: break;
		case 0x6F: break;

		case 0x70: _wave->Write(0, value); break;
		case 0x71: break;
		case 0x72: _wave->Write(1, value); break;
		case 0x73: _wave->Write(2, value); break;
		case 0x74: _wave->Write(3, value); break;
		case 0x75: _wave->Write(4, value); break;
		case 0x76: break;
		case 0x77: break;

		case 0x78: _noise->Write(1, value); break;
		case 0x79: _noise->Write(2, value); break;
		case 0x7A: break;
		case 0x7B: break;
		case 0x7C: _noise->Write(3, value); break;
		case 0x7D: _noise->Write(4, value); break;
		case 0x7E: break;
		case 0x7F: break;

		case 0x80:
			_state.LeftVolume = (value & 0x70) >> 4;
			_state.RightVolume = (value & 0x07);
			break;

		case 0x81:
			_state.EnabledGb = value;
			_state.EnableLeftNoise = (value & 0x80) ? 0xFF : 0;
			_state.EnableLeftWave = (value & 0x40) ? 0xFF : 0;
			_state.EnableLeftSq2 = (value & 0x20) ? 0xFF : 0;
			_state.EnableLeftSq1 = (value & 0x10) ? 0xFF : 0;

			_state.EnableRightNoise = (value & 0x08) ? 0xFF : 0;
			_state.EnableRightWave = (value & 0x04) ? 0xFF : 0;
			_state.EnableRightSq2 = (value & 0x02) ? 0xFF : 0;
			_state.EnableRightSq1 = (value & 0x01) ? 0xFF : 0;
			break;

		case 0x82:
			_state.VolumeControl = value & 0x0F;
			_state.GbVolume = (value & 0x03);
			_state.VolumeA = (value & 0x04) >> 2;
			_state.VolumeB = (value & 0x08) >> 3;
			break;

		case 0x83:
			_state.EnableRightA = value & 0x01;
			_state.EnableLeftA = value & 0x02;
			_state.TimerA = (value & 0x04) >> 2;
			if(value & 0x08) {
				_fifo[0].Clear();
			}

			_state.EnableRightB = value & 0x10;
			_state.EnableLeftB = value & 0x20;
			_state.TimerB = (value & 0x40) >> 6;
			if(value & 0x80) {
				_fifo[1].Clear();
			}

			_state.DmaSoundControl = value & 0x77;
			break;

		case 0x84: {
			bool apuEnabled = value & 0x80;
			if(_state.ApuEnabled != apuEnabled) {
				_state.ApuEnabled = apuEnabled;
				if(!_state.ApuEnabled) {
					_square1->Disable();
					_square2->Disable();
					_wave->Disable();
					_noise->Disable();
					WriteRegister({}, 0x80, 0);
					WriteRegister({}, 0x81, 0);
					_fifo[0].Clear();
					_fifo[1].Clear();
				} else {
					_square1->Disable();
					_square2->Disable();
					_noise->Disable();
					_wave->Disable();
					_square1->ResetLengthCounter();
					_square2->ResetLengthCounter();
					_wave->ResetLengthCounter();
					_noise->ResetLengthCounter();
					_powerOnCycle = _memoryManager->GetMasterClock() / 4;
				}
			}
			break;
		}

		case 0x85:
		case 0x86:
		case 0x87:
			break;

		case 0x88:
			_state.Bias = (_state.Bias & 0x300) | (value & 0xFE);
			break;

		case 0x89:
			_state.Bias = (_state.Bias & 0xFE) | ((value & 0x03) << 8);

			if(_sampleCount) {
				_soundMixer->PlayAudioBuffer(_soundBuffer, _sampleCount / 2, _sampleRate);
				_sampleCount = 0;
			}

			_state.SamplingRate = (value >> 6) & 0x03;
			UpdateSampleRate();
			break;

		case 0x8A: break;
		case 0x8B: break;

		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
		case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E: case 0x9F:
			_wave->WriteRam(addr, value);
			break;

		case 0xA0: case 0xA1: case 0xA2: case 0xA3:
			if(_state.ApuEnabled) {
				bool commit = (addr & 0x03) == 0x03 || (mode & GbaAccessMode::Byte) || ((mode & GbaAccessMode::HalfWord) && (addr & 0x01));
				_fifo[0].Push(value, addr & 0x03, commit);
			}
			break;

		case 0xA4: case 0xA5: case 0xA6: case 0xA7:
			if(_state.ApuEnabled) {
				bool commit = (addr & 0x03) == 0x03 || (mode & GbaAccessMode::Byte) || ((mode & GbaAccessMode::HalfWord) && (addr & 0x01));
				_fifo[1].Push(value, addr & 0x03, commit);
			}
			break;

		default:
			LogDebug("Write unknown sound register: " + HexUtilities::ToHex32(addr) + " = " + HexUtilities::ToHex(value));
			break;
	}
}

void GbaApu::ClockFifo(uint8_t timerIndex)
{
	if(!_state.ApuEnabled) {
		return;
	}

	Run();

	if(_state.TimerA == timerIndex) {
		if(_fifo[0].Size() <= 3) {
			_dmaController->TriggerDmaChannel(GbaDmaTrigger::Special, 1);
		}

		if(!_fifo[0].Empty()) {
			_state.DmaSampleA = (int8_t)_fifo[0].Pop();
		}
	}

	if(_state.TimerB == timerIndex) {
		if(_fifo[1].Size() <= 3) {
			_dmaController->TriggerDmaChannel(GbaDmaTrigger::Special, 2);
		}

		if(!_fifo[1].Empty()) {
			_state.DmaSampleB = (int8_t)_fifo[1].Pop();
		}
	}
}

void GbaApu::UpdateEnabledChannels()
{
	_enabledChannels = (
		(_noise->Enabled() ? 0x08 : 0) |
		(_wave->Enabled() ? 0x04 : 0) |
		(_square2->Enabled() ? 0x02 : 0) |
		(_square1->Enabled() ? 0x01 : 0)
	);
}

void GbaApu::UpdateSampleRate()
{
	_sampleRate = (32 * 1024) << _state.SamplingRate;

	//Used to remove DC offset from audio signal
	_filterL.SetCutoffFrequency(20, _sampleRate);
	_filterR.SetCutoffFrequency(20, _sampleRate);
}

bool GbaApu::IsOddApuCycle()
{
	return (((_memoryManager->GetMasterClock() / 4) - _powerOnCycle) & 0x02) != 0;
}

uint64_t GbaApu::GetElapsedApuCycles()
{
	return (_memoryManager->GetMasterClock() / 4) - _powerOnCycle;
}

GbaApuDebugState GbaApu::GetState()
{
	GbaApuDebugState state;
	state.Common = _state;
	state.Square1 = _square1->GetState();
	state.Square2 = _square2->GetState();
	state.Wave = _wave->GetState();
	state.Noise = _noise->GetState();
	return state;
}

void GbaApu::Serialize(Serializer& s)
{
	if(s.IsSaving()) {
		Run();
		PlayQueuedAudio();
	} else {
		_sampleCount = 0;
	}

	SV(_state.DmaSampleA);
	SV(_state.DmaSampleB);

	SV(_state.VolumeControl);
	SV(_state.GbVolume);
	SV(_state.VolumeA);
	SV(_state.VolumeB);

	SV(_state.DmaSoundControl);
	SV(_state.EnableRightA);
	SV(_state.EnableLeftA);
	SV(_state.TimerA);
	SV(_state.EnableRightB);
	SV(_state.EnableLeftB);
	SV(_state.TimerB);

	SV(_state.EnabledGb);
	SV(_state.EnableLeftSq1);
	SV(_state.EnableLeftSq2);
	SV(_state.EnableLeftWave);
	SV(_state.EnableLeftNoise);

	SV(_state.EnableRightSq1);
	SV(_state.EnableRightSq2);
	SV(_state.EnableRightWave);
	SV(_state.EnableRightNoise);

	SV(_state.LeftVolume);
	SV(_state.RightVolume);

	SV(_state.FrameSequenceStep);
	
	SV(_state.ApuEnabled);

	SV(_state.Bias);
	SV(_state.SamplingRate);

	SV(_prevClockCount);
	SV(_enabledChannels);
	SV(_leftSample);
	SV(_rightSample);

	SV(_fifo[0]);
	SV(_fifo[1]);
	SV(_filterL);
	SV(_filterR);

	SV(_powerOnCycle);

	SV(_square1);
	SV(_square2);
	SV(_wave);
	SV(_noise);

	if(!s.IsSaving()) {
		UpdateSampleRate();
	}
}
