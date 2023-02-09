#pragma once
#include "pch.h"
#include "NES/NesConsole.h"
#include "NES/APU/NesApu.h"
#include "NES/APU/BaseExpansionAudio.h"
#include "Utilities/Serializer.h"

class Sunsoft5bAudio : public BaseExpansionAudio
{
private:
	uint8_t _volumeLut[0x10] = {};
	uint8_t _currentRegister = 0;
	uint8_t _registers[0x10] = {};
	int16_t _lastOutput = 0;
	int16_t _timer[3] = {};
	uint8_t _toneStep[3] = {};
	bool _processTick = false;

	uint16_t GetPeriod(int channel)
	{
		return _registers[channel * 2] | (_registers[channel * 2 + 1] << 8);
	}

	uint16_t GetEnvelopePeriod()
	{
		return _registers[0x0B] | (_registers[0x0C] << 8);
	}

	uint8_t GetNoisePeriod()
	{
		return _registers[6];
	}

	uint8_t GetVolume(int channel)
	{
		return _volumeLut[_registers[8 + channel] & 0x0F];
	}
	
	bool IsEnvelopeEnabled(int channel)
	{
		return (_registers[8 + channel] & 0x10) == 0x10;
	}

	bool IsToneEnabled(int channel)
	{
		return ((_registers[7] >> channel) & 0x01) == 0x00;
	}

	bool IsNoiseEnabled(int channel)
	{
		return ((_registers[7] >> (channel + 3)) & 0x01) == 0x00;
	}
	
	void UpdateChannel(int channel)
	{
		_timer[channel]--;
		if(_timer[channel] <= 0) {
			_timer[channel] = GetPeriod(channel);
			_toneStep[channel] = (_toneStep[channel] + 1) & 0x0F;
		}
	}

	void UpdateOutputLevel()
	{
		int16_t summedOutput = 0;
		for(int i = 0; i < 3; i++) {
			if(IsToneEnabled(i) && _toneStep[i] < 0x08) {
				summedOutput += GetVolume(i);
			}
		}

		_console->GetApu()->AddExpansionAudioDelta(AudioChannel::Sunsoft5B, summedOutput - _lastOutput);
		_lastOutput = summedOutput;
	}

protected:
	void Serialize(Serializer& s) override
	{
		BaseExpansionAudio::Serialize(s);

		SVArray(_timer, 3);
		SVArray(_registers, 0x10);
		SVArray(_toneStep, 3);
		SV(_currentRegister); SV(_lastOutput); SV(_processTick);
	}

	void ClockAudio() override
	{
		if(_processTick) {
			for(int i = 0; i < 3; i++) {
				UpdateChannel(i);
			}
			UpdateOutputLevel();
		}
		_processTick = !_processTick;
	}

public:
	Sunsoft5bAudio(NesConsole* console) : BaseExpansionAudio(console)
	{
		memset(_timer, 0, sizeof(_timer));
		memset(_registers, 0, sizeof(_registers));
		memset(_toneStep, 0, sizeof(_toneStep));
		_currentRegister = 0;
		_lastOutput = 0;
		_processTick = false;

		double output = 1.0;
		_volumeLut[0] = 0;
		for(int i = 1; i < 0x10; i++) {
			//+1.5 dB 2x for every 1 step in volume
			output *= 1.1885022274370184377301224648922;
			output *= 1.1885022274370184377301224648922;

			_volumeLut[i] = (uint8_t)output;
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value)
	{
		switch(addr & 0xE000) {
			case 0xC000:
				_currentRegister = value;
				break;

			case 0xE000:
				if(_currentRegister <= 0x0F) {
					_registers[_currentRegister] = value;
				}
				break;
		}
	}
};