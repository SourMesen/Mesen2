#include "pch.h"
#include "PCE/CdRom/PceAudioFader.h"
#include "PCE/PceConsole.h"
#include "Utilities/Serializer.h"

PceAudioFader::PceAudioFader(PceConsole* console)
{
	_console = console;
}

uint8_t PceAudioFader::Read()
{
	return _state.RegValue;
}

void PceAudioFader::Write(uint8_t value)
{
	_state.RegValue = value;
	_state.Enabled = (value & 0x08) != 0;
	_state.Target = (value & 0x02) ? PceAudioFaderTarget::Adpcm : PceAudioFaderTarget::CdAudio;
	_state.StartClock = _console->GetMasterClock();
	_state.FastFade = (value & 0x04) != 0;
}

double PceAudioFader::GetVolume(PceAudioFaderTarget target)
{
	if(_state.Enabled && _state.Target == target) {
		//Number of clocks to reduce volume by 1%
		uint64_t fadeSpeed = (_state.FastFade ? 0.025 : 0.06) * _console->GetMasterClockRate();

		double volume = 1.0 - (_console->GetMasterClock() - _state.StartClock) / fadeSpeed / 100.0;
		return volume < 0 ? 0 : volume;
	}
	return 1.0;
}

void PceAudioFader::Serialize(Serializer& s)
{
	SV(_state.Enabled);
	SV(_state.Target);
	SV(_state.StartClock);
	SV(_state.FastFade);
	SV(_state.RegValue);
}