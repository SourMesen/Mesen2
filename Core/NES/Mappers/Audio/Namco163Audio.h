#pragma once
#include "pch.h"
#include "NES/NesConsole.h"
#include "NES/APU/NesApu.h"
#include "NES/APU/BaseExpansionAudio.h"
#include "Utilities/Serializer.h"

class Namco163Audio : public BaseExpansionAudio
{
public:
	static constexpr uint32_t AudioRamSize = 0x80;

private:
	uint8_t _internalRam[Namco163Audio::AudioRamSize] = {};
	int16_t _channelOutput[8] = {};
	uint8_t _ramPosition = 0;
	bool _autoIncrement = false;
	uint8_t _updateCounter = 0;
	int8_t _currentChannel = 0;
	int16_t _lastOutput = 0;
	bool _disableSound = false;

	enum SoundReg
	{
		FrequencyLow = 0x00,
		PhaseLow = 0x01,
		FrequencyMid = 0x02,
		PhaseMid = 0x03,
		FrequencyHigh = 0x04,
		WaveLength = 0x04,
		PhaseHigh = 0x05,
		WaveAddress = 0x06,
		Volume = 0x07
	};

	uint32_t GetFrequency(int channel)
	{
		uint8_t baseAddr = 0x40 + channel * 0x08;
		return ((_internalRam[baseAddr + SoundReg::FrequencyHigh] & 0x03) << 16) | (_internalRam[baseAddr + SoundReg::FrequencyMid] << 8) | _internalRam[baseAddr + SoundReg::FrequencyLow];
	}

	uint32_t GetPhase(int channel)
	{
		uint8_t baseAddr = 0x40 + channel * 0x08;
		return (_internalRam[baseAddr + SoundReg::PhaseHigh] << 16) | (_internalRam[baseAddr + SoundReg::PhaseMid] << 8) | _internalRam[baseAddr + SoundReg::PhaseLow];
	}

	void SetPhase(int channel, uint32_t phase)
	{
		uint8_t baseAddr = 0x40 + channel * 0x08;
		_internalRam[baseAddr + SoundReg::PhaseHigh] = (phase >> 16) & 0xFF;
		_internalRam[baseAddr + SoundReg::PhaseMid] = (phase >> 8) & 0xFF;
		_internalRam[baseAddr + SoundReg::PhaseLow] = phase & 0xFF;
	}

	uint8_t GetWaveAddress(int channel)
	{
		uint8_t baseAddr = 0x40 + channel * 0x08;
		return _internalRam[baseAddr + SoundReg::WaveAddress];
	}

	uint16_t GetWaveLength(int channel)
	{
		uint8_t baseAddr = 0x40 + channel * 0x08;
		return 256 - (_internalRam[baseAddr + SoundReg::WaveLength] & 0xFC);
	}

	uint8_t GetVolume(int channel)
	{
		uint8_t baseAddr = 0x40 + channel * 0x08;
		return _internalRam[baseAddr + SoundReg::Volume] & 0x0F;
	}
	
	uint8_t GetNumberOfChannels()
	{
		return (_internalRam[0x7F] >> 4) & 0x07;
	}

	void UpdateChannel(int channel)
	{
		uint32_t phase = GetPhase(channel);
		uint32_t freq = GetFrequency(channel);
		uint16_t length = GetWaveLength(channel);
		uint8_t offset = GetWaveAddress(channel);
		uint8_t volume = GetVolume(channel);

		phase = (phase + freq) % (length << 16);
		
		uint8_t samplePosition = ((phase >> 16) + offset) & 0xFF;
		int8_t sample;
		if((samplePosition & 0x01)) {
			sample = _internalRam[samplePosition / 2] >> 4;
		} else {
			sample = _internalRam[samplePosition / 2] & 0x0F;
		}

		_channelOutput[channel] = (sample - 8) * volume;
		UpdateOutputLevel();
		SetPhase(channel, phase);
	}

	void UpdateOutputLevel()
	{
		int16_t summedOutput = 0;
		for(int i = 7, min = 7 - GetNumberOfChannels(); i >= min; i--) {
			summedOutput += _channelOutput[i];
		}
		summedOutput /= GetNumberOfChannels() + 1;

		_console->GetApu()->AddExpansionAudioDelta(AudioChannel::Namco163, summedOutput - _lastOutput);
		_lastOutput = summedOutput;
	}

protected:
	void Serialize(Serializer& s) override
	{
		BaseExpansionAudio::Serialize(s);

		SVArray(_internalRam, 0x80);
		SVArray(_channelOutput, 8);
		SV(_ramPosition); SV(_autoIncrement); SV(_updateCounter); SV(_currentChannel); SV(_lastOutput); SV(_disableSound);
	}

	void ClockAudio() override
	{
		if(!_disableSound) {
			_updateCounter++;
			if(_updateCounter == 15) {
				UpdateChannel(_currentChannel);

				_updateCounter = 0;
				_currentChannel--;
				if(_currentChannel < 7 - GetNumberOfChannels()) {
					_currentChannel = 7;
				}
			}
		}
	}

public:
	Namco163Audio(NesConsole* console) : BaseExpansionAudio(console)
	{
		console->InitializeRam(_internalRam, Namco163Audio::AudioRamSize);
		memset(_channelOutput, 0, sizeof(_channelOutput));
		_ramPosition = 0;
		_autoIncrement = false;
		_updateCounter = 0;
		_currentChannel = 7;
		_lastOutput = 0;
		_disableSound = false;
	}

	uint8_t* GetInternalRam()
	{
		return _internalRam;
	}

	void WriteRegister(uint16_t addr, uint8_t value)
	{
		switch(addr & 0xF800) {
			case 0x4800:
				_internalRam[_ramPosition] = value;
				if(_autoIncrement) {
					_ramPosition = (_ramPosition + 1) & 0x7F;
				}
				break;
			case 0xE000:
				_disableSound = (value & 0x40) == 0x40;
				break;
			case 0xF800:
				_ramPosition = value & 0x7F;
				_autoIncrement = (value & 0x80) == 0x80;
				break;

		}
	}

	uint8_t ReadRegister(uint16_t addr)
	{
		uint8_t value = 0;
		switch(addr & 0xF800) {
			case 0x4800: {
				value = _internalRam[_ramPosition];
				if(_autoIncrement) {
					_ramPosition = (_ramPosition + 1) & 0x7F;
				}
				break;
			}
		}

		return value;
	}
};