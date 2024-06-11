#include "pch.h"
#include "SMS/SmsFmAudio.h"
#include "SMS/SmsConsole.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Audio/SoundMixer.h"
#include "Shared/Utilities/emu2413.h"
#include "Shared/Utilities/Emu2413Serializer.h"
#include "Utilities/Serializer.h"

SmsFmAudio::SmsFmAudio(Emulator* emu, SmsConsole* console)
{
	_emu = emu;
	_console = console;

	_opll = OPLL_new(_console->GetMasterClockRate(), _console->GetMasterClockRate() / 72);
	OPLL_setChipType(_opll, 0);
	OPLL_resetPatch(_opll, 0);
	OPLL_reset(_opll);
	_emu->GetSoundMixer()->RegisterAudioProvider(this);
}

SmsFmAudio::~SmsFmAudio()
{
	OPLL_delete(_opll);
	_emu->GetSoundMixer()->UnregisterAudioProvider(this);
}

void SmsFmAudio::Run()
{
	if(_fmEnabled && _emu->GetSettings()->GetSmsConfig().EnableFmAudio) {
		uint64_t clocksToRun = _console->GetMasterClock() - _prevMasterClock;
		while(clocksToRun >= 72) {
			int16_t output = OPLL_calc(_opll);
			_samplesToPlay.push_back(output);
			_samplesToPlay.push_back(output);
			_prevMasterClock += 72;
			clocksToRun -= 72;
		}
	} else {
		_prevMasterClock = _console->GetMasterClock();
	}
}

bool SmsFmAudio::IsPsgAudioMuted()
{
	//PSG is muted when 1 or 2
	//This only works on the Japanese SMS - not on Mark III consoles
	return _audioControl == 0x01 || _audioControl == 0x02;
}

uint8_t SmsFmAudio::Read()
{
	//TODOSMS - c-sync counter bits?
	return _audioControl & 0x03;
}

void SmsFmAudio::Write(uint8_t port, uint8_t value)
{
	switch(port) {
		case 0xF0:
		case 0xF1:
			_fmEnabled = true;
			Run();
			OPLL_writeIO(_opll, port, value);
			break;

		case 0xF2:
			_audioControl = value & 0x03;
			break;
	}
}

void SmsFmAudio::MixAudio(int16_t* out, uint32_t sampleCount, uint32_t sampleRate)
{
	Run();

	if(_audioControl == 0 || _audioControl == 2) {
		//FM audio is muted when 0 or 2
		_samplesToPlay.clear();
		return;
	}

	_resampler.SetSampleRates(_console->GetMasterClockRate() / 72.0, sampleRate);
	_resampler.SetVolume(_emu->GetSettings()->GetSmsConfig().FmAudioVolume * 1.5 / 100.0);
	_resampler.Resample<true>(_samplesToPlay.data(), (uint32_t)_samplesToPlay.size() / 2, out, sampleCount, true);
	_samplesToPlay.clear();
}

void SmsFmAudio::Serialize(Serializer& s)
{
	SV(_prevMasterClock);
	SV(_audioControl);
	SV(_fmEnabled);
	Emu2413Serializer::Serialize(_opll, s);
}
