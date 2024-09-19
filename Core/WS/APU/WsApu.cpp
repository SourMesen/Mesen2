#include "pch.h"
#include "WS/APU/WsApu.h"
#include "WS/APU/WsApuCh1.h"
#include "WS/APU/WsApuCh2.h"
#include "WS/APU/WsApuCh3.h"
#include "WS/APU/WsApuCh4.h"
#include "WS/APU/WsHyperVoice.h"
#include "WS/WsConsole.h"
#include "WS/WsPpu.h"
#include "WS/WsDmaController.h"
#include "WS/WsMemoryManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Audio/SoundMixer.h"
#include "Utilities/BitUtilities.h"
#include "Utilities/Serializer.h"

WsApu::WsApu(Emulator* emu, WsConsole* console, WsMemoryManager* memoryManager, WsDmaController* dmaController)
{
	_emu = emu;
	_console = console;
	_memoryManager = memoryManager;
	_dmaController = dmaController;
	_soundMixer = emu->GetSoundMixer();

	_state.MasterVolume = _console->GetModel() == WsModel::Monochrome ? 2 : 3;
	_state.InternalMasterVolume = _state.MasterVolume;

	_ch1.reset(new WsApuCh1(this, _state.Ch1));
	_ch2.reset(new WsApuCh2(this, _state.Ch2));
	_ch3.reset(new WsApuCh3(this, _state.Ch3));
	_ch4.reset(new WsApuCh4(this, _state.Ch4));
	_hyperVoice.reset(new WsHyperVoice(_state.Voice));

	_soundBuffer = new int16_t[WsApu::MaxSamples * 2];
	memset(_soundBuffer, 0, WsApu::MaxSamples * 2 * sizeof(int16_t));

	_filterL.SetCutoffFrequency(16, WsApu::ApuFrequency);
	_filterR.SetCutoffFrequency(16, WsApu::ApuFrequency);
}

WsApu::~WsApu()
{
	delete[] _soundBuffer;
}

void WsApu::ChangeMasterVolume()
{
	if(_emu->GetSettings()->GetWsConfig().AudioMode == WsAudioMode::Speakers) {
		if(_state.InternalMasterVolume == 0) {
			_state.InternalMasterVolume = _console->GetModel() == WsModel::Monochrome ? 2 : 3;
		} else {
			_state.InternalMasterVolume--;
		}
	}
	_console->GetPpu()->ShowVolumeIcon();
}

void WsApu::PlayQueuedAudio()
{
	_soundMixer->PlayAudioBuffer(_soundBuffer, _sampleCount, WsApu::ApuFrequency);
	_sampleCount = 0;
	_clockCounter = 0;
}

void WsApu::WriteDma(bool forHyperVoice, uint8_t sampleValue)
{
	if(forHyperVoice) {
		_hyperVoice->WriteDma(sampleValue);
	} else {
		_state.Ch2.RightVolume = sampleValue & 0x0F;
		_state.Ch2.LeftVolume = sampleValue >> 4;
	}
}

uint8_t WsApu::ReadSample(uint8_t ch, uint8_t pos)
{
	//TODOWS review once the exact behavior for these 2 flags is understood
	if(_state.ForceOutput4) {
		return 4;
	} else if(_state.ForceOutput2) {
		return 2;
	}

	uint32_t addr = _state.WaveTableAddress + (ch * 16) + (pos >> 1);
	uint8_t value = _memoryManager->InternalRead(addr);
	_emu->ProcessMemoryRead<CpuType::Ws>(addr, value, MemoryOperationType::Read);
	if(pos & 0x01) {
		return value & 0x0F;
	} else {
		return value >> 4;
	}
}

void WsApu::UpdateOutput()
{
	WsConfig& cfg = _emu->GetSettings()->GetWsConfig();

	int32_t leftOutput = (
		_state.Ch1.LeftOutput * cfg.Channel1Vol / 100 +
		_state.Ch2.LeftOutput * cfg.Channel2Vol / 100 +
		_state.Ch3.LeftOutput * cfg.Channel3Vol / 100 +
		_state.Ch4.LeftOutput * cfg.Channel4Vol / 100
	);

	int32_t rightOutput = (
		_state.Ch1.RightOutput * cfg.Channel1Vol / 100 +
		_state.Ch2.RightOutput * cfg.Channel2Vol / 100 +
		_state.Ch3.RightOutput * cfg.Channel3Vol / 100 +
		_state.Ch4.RightOutput * cfg.Channel4Vol / 100
	);

	if(_state.ForceOutputCh2Voice) {
		leftOutput = (_state.Ch2.GetVolume() * 5) & 0x3FF;
		rightOutput = leftOutput;
	}
	
	if(cfg.AudioMode == WsAudioMode::Headphones) {
		if(_state.HeadphoneEnabled) {
			leftOutput <<= 5;
			rightOutput <<= 5;

			if(_state.Voice.Enabled) {
				leftOutput += _state.Voice.LeftOutput * cfg.Channel5Vol / 100;
				rightOutput += _state.Voice.RightOutput * cfg.Channel5Vol / 100;
			}
		} else {
			leftOutput = 0;
			rightOutput = 0;
		}
	} else {
		if(_state.SpeakerEnabled) {
			int32_t out = (((leftOutput + rightOutput) >> _state.SpeakerVolume) & 0xFF) << 7;
			leftOutput = out;
			rightOutput = out;

			if(_console->GetModel() == WsModel::Monochrome) {
				switch(_state.InternalMasterVolume) {
					case 0: leftOutput = 0; rightOutput = 0; break;
					case 1: leftOutput >>= 1; rightOutput >>= 1; break;
					case 2: break;
				}
			} else {
				switch(_state.InternalMasterVolume) {
					case 0: leftOutput = 0; rightOutput = 0; break;
					case 1: leftOutput /= 3; rightOutput /= 3; break;
					case 2: leftOutput = leftOutput * 2 / 3; rightOutput = rightOutput * 2 / 3; break;
					case 3: break;
				}
			}
		} else {
			leftOutput = 0;
			rightOutput = 0;
		}
	}

	leftOutput = std::clamp<int32_t>(leftOutput, INT16_MIN, INT16_MAX);
	rightOutput = std::clamp<int32_t>(rightOutput, INT16_MIN, INT16_MAX);

	//Use low pass filter and subtract the result to filter out DC offset
	_soundBuffer[_sampleCount*2] = leftOutput - _filterL.Process(leftOutput);
	_soundBuffer[_sampleCount*2+1] = rightOutput - _filterR.Process(rightOutput);
	_sampleCount++;

	if(_sampleCount >= WsApu::MaxSamples) {
		PlayQueuedAudio();
	}
}

void WsApu::Run()
{
	_clockCounter++;

	_ch1->Exec();
	_ch2->Exec();
	_ch3->Exec();
	_ch4->Exec();

	uint8_t clock = (_clockCounter - 1) & 0x7F;
	if(clock >= 116) {
		switch(clock) {
			case 116: _dmaController->ProcessSoundDma(); break;
			case 122: _hyperVoice->Exec(); break;
			case 123: _ch1->UpdateOutput(); break;
			case 124: _ch2->UpdateOutput(); break;
			case 125: _ch3->UpdateOutput(); break;
			case 126: _ch4->UpdateOutput(); break;
			case 127: UpdateOutput(); break;
		}
	}
}

uint8_t WsApu::Read(uint16_t port)
{
	switch(port) {
		case 0x6A: 
		case 0x6B:
			return _hyperVoice->Read(port);

		case 0x80: return BitUtilities::GetBits<0>(_state.Ch1.Frequency);
		case 0x81: return BitUtilities::GetBits<8>(_state.Ch1.Frequency);
		case 0x82: return BitUtilities::GetBits<0>(_state.Ch2.Frequency);
		case 0x83: return BitUtilities::GetBits<8>(_state.Ch2.Frequency);
		case 0x84: return BitUtilities::GetBits<0>(_state.Ch3.Frequency);
		case 0x85: return BitUtilities::GetBits<8>(_state.Ch3.Frequency);
		case 0x86: return BitUtilities::GetBits<0>(_state.Ch4.Frequency);
		case 0x87: return BitUtilities::GetBits<8>(_state.Ch4.Frequency);

		case 0x88: return _state.Ch1.GetVolume();
		case 0x89: return _state.Ch2.GetVolume();
		case 0x8A: return _state.Ch3.GetVolume();
		case 0x8B: return _state.Ch4.GetVolume();

		case 0x8C: return (uint8_t)_state.Ch3.SweepValue;
		case 0x8D: return _state.Ch3.SweepPeriod;
		case 0x8E:
			return (
				_state.Ch4.TapMode |
				((uint8_t)_state.Ch4.LfsrEnabled << 4)
			);

		case 0x8F: return _state.WaveTableAddress >> 6;

		case 0x90:
			return (
				(uint8_t)_state.Ch1.Enabled |
				((uint8_t)_state.Ch2.Enabled << 1) |
				((uint8_t)_state.Ch3.Enabled << 2) |
				((uint8_t)_state.Ch4.Enabled << 3) |
				((uint8_t)_state.Ch2.PcmEnabled << 5) |
				((uint8_t)_state.Ch3.SweepEnabled << 6) |
				((uint8_t)_state.Ch4.NoiseEnabled << 7)
			);

		case 0x91:
			return (
				(uint8_t)_state.SpeakerEnabled |
				(_state.SpeakerVolume << 1) |
				((uint8_t)_state.HeadphoneEnabled << 3) |
				(_emu->GetSettings()->GetWsConfig().AudioMode == WsAudioMode::Headphones ? 0x80 : 0)
			);

		case 0x92: return BitUtilities::GetBits<0>(_state.Ch4.Lfsr);
		case 0x93: return BitUtilities::GetBits<8>(_state.Ch4.Lfsr);

		case 0x94:
			return (
				(uint8_t)_state.Ch2.MaxPcmVolumeRight |
				((uint8_t)_state.Ch2.HalfPcmVolumeRight << 1) |
				((uint8_t)_state.Ch2.MaxPcmVolumeLeft << 2) |
				((uint8_t)_state.Ch2.HalfPcmVolumeLeft << 3)
			);

		case 0x95:
			return (
				(uint8_t)_state.HoldChannels |
				((uint8_t)_state.Ch3.UseSweepCpuClock << 1) |
				(_state.Ch4.HoldLfsr << 2) |
				//todows bit 4?
				((uint8_t)_state.ForceOutputCh2Voice << 5) |
				((uint8_t)_state.ForceOutput2 << 6) |
				((uint8_t)_state.ForceOutput4 << 7)
			);

		case 0x96: return GetApuOutput(true);
		case 0x97: return GetApuOutput(true) >> 8;
		case 0x98: return GetApuOutput(false);
		case 0x99: return GetApuOutput(false) >> 8;
		case 0x9A: return GetApuOutput(false) + GetApuOutput(true);
		case 0x9B: return (GetApuOutput(false) + GetApuOutput(true)) >> 8;

		case 0x9E:
			if(_console->GetModel() != WsModel::Monochrome) {
				return _state.MasterVolume;
			}
			break;
	}

	//todows open bus
	return 0x90;
}

void WsApu::Write(uint16_t port, uint8_t value)
{
	switch(port) {
		case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x69: case 0x6A: case 0x6B:
			_hyperVoice->Write(port, value);
			break;

		case 0x80: BitUtilities::SetBits<0>(_state.Ch1.Frequency, value); break;
		case 0x81: BitUtilities::SetBits<8>(_state.Ch1.Frequency, value & 0x0F); break;
		case 0x82: BitUtilities::SetBits<0>(_state.Ch2.Frequency, value); break;
		case 0x83: BitUtilities::SetBits<8>(_state.Ch2.Frequency, value & 0x0F); break;
		case 0x84: BitUtilities::SetBits<0>(_state.Ch3.Frequency, value); break;
		case 0x85: BitUtilities::SetBits<8>(_state.Ch3.Frequency, value & 0x0F); break;
		case 0x86: BitUtilities::SetBits<0>(_state.Ch4.Frequency, value); break;
		case 0x87: BitUtilities::SetBits<8>(_state.Ch4.Frequency, value & 0x0F); break;

		case 0x88: _state.Ch1.SetVolume(value); break;
		case 0x89: _state.Ch2.SetVolume(value); break;
		case 0x8A: _state.Ch3.SetVolume(value); break;
		case 0x8B: _state.Ch4.SetVolume(value); break;

		case 0x8C: _state.Ch3.SweepValue = (int8_t)value; break;
		case 0x8D: _state.Ch3.SweepPeriod = value & 0x1F; break;
		
		case 0x8E: {
			static constexpr uint8_t tapShifts[8] = { 14, 10, 13, 4, 8, 6, 9, 11 };
			_state.Ch4.TapMode = value & 0x07;
			_state.Ch4.TapShift = tapShifts[_state.Ch4.TapMode];
			if(value & 0x08) {
				_state.Ch4.Lfsr = 0;
			}
			_state.Ch4.LfsrEnabled = value & 0x10;
			break;
		}

		case 0x8F:
			_state.WaveTableAddress = value << 6;
			break;

		case 0x90:
			_state.Ch1.Enabled = value & 0x01;
			_state.Ch2.Enabled = value & 0x02;
			_state.Ch3.Enabled = value & 0x04;
			_state.Ch4.Enabled = value & 0x08;

			_state.Ch2.PcmEnabled = value & 0x20;
			_state.Ch3.SweepEnabled = value & 0x40;
			_state.Ch4.NoiseEnabled = value & 0x80;
			break;

		case 0x91:
			_state.SpeakerEnabled = value & 0x01;
			_state.SpeakerVolume = (value & 0x06) >> 1;
			_state.HeadphoneEnabled = value & 0x08;
			break;

		case 0x94:
			_state.Ch2.MaxPcmVolumeRight = value & 0x01;
			_state.Ch2.HalfPcmVolumeRight = value & 0x02;
			_state.Ch2.MaxPcmVolumeLeft = value & 0x04;
			_state.Ch2.HalfPcmVolumeLeft = value & 0x08;
			break;

		case 0x95:
			_state.HoldChannels = value & 0x01;
			_state.Ch3.UseSweepCpuClock = value & 0x02;
			_state.Ch4.HoldLfsr = (value & 0x0C) >> 2;
			//todows bit 4?
			_state.ForceOutputCh2Voice = value & 0x20;
			_state.ForceOutput2 = value & 0x40;
			_state.ForceOutput4 = value & 0x80;
			break;

		case 0x9E:
			if(_console->GetModel() != WsModel::Monochrome) {
				_state.InternalMasterVolume = value & 0x03;
				_state.MasterVolume = value & 0x03;
			}
			break;
	}
}

uint16_t WsApu::GetApuOutput(bool forRight)
{
	if(forRight) {
		return (
			_state.Ch1.RightOutput +
			_state.Ch2.RightOutput +
			_state.Ch3.RightOutput +
			_state.Ch4.RightOutput
		);
	} else {
		return (
			_state.Ch1.LeftOutput +
			_state.Ch2.LeftOutput +
			_state.Ch3.LeftOutput +
			_state.Ch4.LeftOutput
		);
	}
}

void WsApu::Serialize(Serializer& s)
{
	SV(_state.Ch1.Frequency);
	SV(_state.Ch1.Timer);
	SV(_state.Ch1.Enabled);
	SV(_state.Ch1.LeftVolume);
	SV(_state.Ch1.RightVolume);
	SV(_state.Ch1.SamplePosition);
	SV(_state.Ch1.LeftOutput);
	SV(_state.Ch1.RightOutput);

	SV(_state.Ch2.Frequency);
	SV(_state.Ch2.Timer);
	SV(_state.Ch2.Enabled);
	SV(_state.Ch2.LeftVolume);
	SV(_state.Ch2.RightVolume);
	SV(_state.Ch2.SamplePosition);
	SV(_state.Ch2.LeftOutput);
	SV(_state.Ch2.RightOutput);
	SV(_state.Ch2.PcmEnabled);
	SV(_state.Ch2.MaxPcmVolumeRight);
	SV(_state.Ch2.HalfPcmVolumeRight);
	SV(_state.Ch2.MaxPcmVolumeLeft);
	SV(_state.Ch2.HalfPcmVolumeLeft);

	SV(_state.Ch3.Frequency);
	SV(_state.Ch3.Timer);
	SV(_state.Ch3.Enabled);
	SV(_state.Ch3.LeftVolume);
	SV(_state.Ch3.RightVolume);
	SV(_state.Ch3.SamplePosition);
	SV(_state.Ch3.LeftOutput);
	SV(_state.Ch3.RightOutput);
	SV(_state.Ch3.SweepScaler);
	SV(_state.Ch3.SweepEnabled);
	SV(_state.Ch3.SweepValue);
	SV(_state.Ch3.SweepPeriod);
	SV(_state.Ch3.SweepTimer);
	SV(_state.Ch3.UseSweepCpuClock);

	SV(_state.Ch4.Frequency);
	SV(_state.Ch4.Timer);
	SV(_state.Ch4.Enabled);
	SV(_state.Ch4.LeftVolume);
	SV(_state.Ch4.RightVolume);
	SV(_state.Ch4.SamplePosition);
	SV(_state.Ch4.LeftOutput);
	SV(_state.Ch4.RightOutput);
	SV(_state.Ch4.NoiseEnabled);
	SV(_state.Ch4.LfsrEnabled);
	SV(_state.Ch4.TapMode);
	SV(_state.Ch4.TapShift);
	SV(_state.Ch4.Lfsr);
	SV(_state.Ch4.HoldLfsr);

	SV(_state.Voice.LeftOutput);
	SV(_state.Voice.RightOutput);
	SV(_state.Voice.Enabled);
	SV(_state.Voice.LeftSample);
	SV(_state.Voice.RightSample);
	SV(_state.Voice.UpdateRightValue);
	SV(_state.Voice.Divisor);
	SV(_state.Voice.Timer);
	SV(_state.Voice.Input);
	SV(_state.Voice.Shift);
	SV(_state.Voice.ChannelMode);
	SV(_state.Voice.ScalingMode);
	SV(_state.Voice.ControlLow);
	SV(_state.Voice.ControlHigh);

	SV(_state.WaveTableAddress);
	SV(_state.SpeakerEnabled);
	SV(_state.SpeakerVolume);
	SV(_state.MasterVolume);
	SV(_state.InternalMasterVolume);
	SV(_state.HeadphoneEnabled);
	SV(_state.HoldChannels);
	SV(_state.ForceOutput2);
	SV(_state.ForceOutput4);
	SV(_state.ForceOutputCh2Voice);

	SV(_filterL);
	SV(_filterR);
}
