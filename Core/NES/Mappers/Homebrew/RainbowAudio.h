#pragma once
#include "NES/APU/BaseExpansionAudio.h"
#include "NES/Mappers/Audio/Vrc6Pulse.h"
#include "NES/Mappers/Audio/Vrc6Saw.h"
#include "NES/APU/NesApu.h"
#include "NES/NesConsole.h"
#include "Utilities/Serializer.h"

class RainbowAudio : public BaseExpansionAudio
{
private:
	Vrc6Pulse _pulse1;
	Vrc6Pulse _pulse2;
	Vrc6Saw _saw;
	bool _outputExpPin6 = false;
	bool _outputExpPin9 = false;
	bool _outputTo4011 = false;
	uint8_t _volume = 0;
	uint8_t _lastOutput = 0;

protected:
	void Serialize(Serializer& s) override
	{
		SV(_pulse1);
		SV(_pulse2);
		SV(_saw);
		SV(_outputExpPin6);
		SV(_outputExpPin9);
		SV(_outputTo4011);
		SV(_volume);
		SV(_lastOutput);
	}

	void ClockAudio() override
	{
		_pulse1.Clock();
		_pulse2.Clock();
		_saw.Clock();

		uint8_t outputLevel = _pulse1.GetVolume() + _pulse2.GetVolume() + _saw.GetVolume();
		if(_outputExpPin6 || _outputExpPin9) {
			_console->GetApu()->AddExpansionAudioDelta(AudioChannel::VRC6, ((int16_t)outputLevel - (int16_t)_lastOutput) * 15 * _volume / 15);
		}
		_lastOutput = outputLevel;
	}

public:
	RainbowAudio(NesConsole* console) : BaseExpansionAudio(console)
	{
		Reset();
	}

	void Reset()
	{
		_lastOutput = 0;
	}

	uint8_t GetLastOutput()
	{
		return _lastOutput;
	}

	void WriteRegister(uint16_t addr, uint8_t value)
	{
		addr &= 0x0F;

		switch(addr) {
			case 0x00: case 0x01: case 0x02:
				_pulse1.WriteReg(addr, value);
				break;

			case 0x03: case 0x04: case 0x05:
				_pulse2.WriteReg(addr - 0x03, value);
				break;

			case 0x06: case 0x07: case 0x08:
				_saw.WriteReg(addr - 0x06, value);
				break;

			case 0x09:
				_outputExpPin6 = value & 0x01;
				_outputExpPin9 = value & 0x02;
				_outputTo4011 = value & 0x04;
				break;

			case 0x0A:
				_volume = value & 0x0F;
				break;
		}
	}
};