#pragma once 
#include "pch.h"
#include "NES/APU/BaseExpansionAudio.h"
#include "NES/APU/NesApu.h"
#include "NES/NesConsole.h"
#include "NES/Mappers/Audio/OpllEmulator.h"
#include "Utilities/Serializer.h"

class Vrc7Audio : public BaseExpansionAudio
{
private:
	unique_ptr<Vrc7Opll::OpllEmulator> _opllEmulator;
	uint8_t _currentReg;
	int16_t _previousOutput;
	double _clockTimer;
	bool _muted;

protected:
	void ClockAudio() override
	{
		if(_clockTimer == 0) {
			_clockTimer = ((double)_console->GetMasterClockRate()) / 49716;
		}

		_clockTimer--;
		if(_clockTimer <= 0) {
			int16_t output = _opllEmulator->GetOutput();
			_console->GetApu()->AddExpansionAudioDelta(AudioChannel::VRC7, _muted ? 0 : (output - _previousOutput));
			_previousOutput = output;
			_clockTimer = ((double)_console->GetMasterClockRate()) / 49716;
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseExpansionAudio::Serialize(s);

		SV(_opllEmulator);
		SV(_currentReg); SV(_previousOutput); SV(_clockTimer); SV(_muted);
	}

public:
	Vrc7Audio(NesConsole* console) : BaseExpansionAudio(console)
	{
		_previousOutput = 0;
		_currentReg = 0;
		_muted = false;
		_clockTimer = 0;
		_opllEmulator.reset(new Vrc7Opll::OpllEmulator());
	}

	void SetMuteAudio(bool muted)
	{
		_muted = muted;
	}

	void WriteReg(uint16_t addr, uint8_t value)
	{
		switch(addr & 0xF030) {
			case 0x9010:
				_currentReg = value;
				break;
			case 0x9030:
				_opllEmulator->WriteReg(_currentReg, value);
				break;
		}
	}
};