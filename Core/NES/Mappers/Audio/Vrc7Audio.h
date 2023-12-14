#pragma once 
#include "pch.h"
#include "NES/APU/BaseExpansionAudio.h"
#include "NES/APU/NesApu.h"
#include "NES/NesConsole.h"
#include "Shared/Utilities/emu2413.h"
#include "Shared/Utilities/Emu2413Serializer.h"
#include "Utilities/Serializer.h"

class Vrc7Audio : public BaseExpansionAudio
{
private:
	static constexpr int OpllSampleRate = 49716;
	static constexpr int OpllClockRate = Vrc7Audio::OpllSampleRate * 72;

	OPLL* _opll = nullptr;
	uint8_t _currentReg = 0;
	int16_t _previousOutput = 0;
	double _clockTimer = 0;
	bool _muted = false;

protected:
	void ClockAudio() override
	{
		if(_clockTimer == 0) {
			_clockTimer = ((double)_console->GetMasterClockRate()) / Vrc7Audio::OpllSampleRate;
		}

		_clockTimer--;
		if(_clockTimer <= 0) {
			int16_t output = OPLL_calc(_opll);
			_console->GetApu()->AddExpansionAudioDelta(AudioChannel::VRC7, _muted ? 0 : (output - _previousOutput));
			_previousOutput = output;
			_clockTimer = ((double)_console->GetMasterClockRate()) / Vrc7Audio::OpllSampleRate;
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseExpansionAudio::Serialize(s);

		SV(_currentReg); SV(_previousOutput); SV(_clockTimer); SV(_muted);
		Emu2413Serializer::Serialize(_opll, s);
	}

public:
	Vrc7Audio(NesConsole* console) : BaseExpansionAudio(console)
	{
		_previousOutput = 0;
		_currentReg = 0;
		_muted = false;
		_clockTimer = 0;
		
		_opll = OPLL_new(Vrc7Audio::OpllClockRate, Vrc7Audio::OpllSampleRate);

		//Set OPLL emulator to VRC7 mode
		OPLL_setChipType(_opll, 1);
		OPLL_resetPatch(_opll, 1);
	}

	~Vrc7Audio()
	{
		OPLL_delete(_opll);
	}

	void Reset()
	{
		OPLL_reset(_opll);
	}

	void SetMuteAudio(bool muted)
	{
		_muted = muted;
	}

	void WriteReg(uint16_t addr, uint8_t value)
	{
		if(_muted) {
			//"Writes to $9010 and $9030 are disregarded while this bit is set."
			return;
		}

		switch(addr & 0xF030) {
			case 0x9010:
				_currentReg = value;
				break;
			case 0x9030:
				OPLL_writeReg(_opll, _currentReg, value);
				break;
		}
	}
};