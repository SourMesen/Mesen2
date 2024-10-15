#include "pch.h"
#include "NES/NesConsole.h"
#include "Shared/Emulator.h"
#include "NES/INesMemoryHandler.h"
#include "NES/Epsm.h"
#include "Utilities/Serializer.h"

Epsm::Epsm(Emulator* emu, NesConsole* console, vector<uint8_t>& adpcmRom) : _opn(console, adpcmRom)
{
	_console = console;
	_emu = emu;
	_masterClockRate = _console->GetMasterClockRate();
	_emu->GetSoundMixer()->RegisterAudioProvider(this);
}

Epsm::~Epsm()
{
	_emu->GetSoundMixer()->UnregisterAudioProvider(this);
}

void Epsm::Write(uint8_t dataBus, uint8_t outPins)
{
	//4016 writes
	//The EPSM uses the value of the data bus + the OUT1 pin
	//The OUT1 can be delayed by a cycle, but not the data bus
	bool isHigh = (outPins & 0x02);
	bool wasHigh = (_prevOutPins & 0x02);

	if(isHigh && !wasHigh) {
		//rising edge
		_data = (dataBus & 0xF0) | (_data & 0x0F);
		_addr = ((dataBus & 0x04) >> 1) | ((dataBus & 0x08) >> 3);
	} else if(!isHigh && wasHigh) {
		//falling edge
		_data = (_data & 0xF0) | ((dataBus & 0xF0) >> 4);
		_opn.Write(_addr, _data);
	}

	_prevOutPins = outPins;
}

void Epsm::WriteRam(uint16_t addr, uint8_t value)
{
	//401C-401F writes
	_opn.Write(addr, value);
}

void Epsm::OnRegionChanged()
{
	uint64_t masterClockRate = _console->GetMasterClockRate();
	if(_masterClockRate != masterClockRate) {
		//Reset alignment between cpu & epsm clocks if the region changes (e.g pal to ntsc, etc.)
		_masterClockRate = masterClockRate;
		_clockCounter = GetTargetClock();
	}
}

uint64_t Epsm::GetTargetClock()
{
	uint64_t masterClock = _console->GetMasterClock();
	double clockRatio = (double)OpnInterface::ClockRate / _masterClockRate;
	return masterClock * clockRatio;
}

void Epsm::Exec()
{
	constexpr int clocksPerSample = 144;

	uint64_t targetClock = GetTargetClock();

	while(_clockCounter < targetClock) {
		_clockCounter++;
		_opn.Exec();

		if(++_sampleClockCounter == clocksPerSample) {
			_opn.GenerateSamples(_samples);
			_sampleClockCounter = 0;
		}
	}
}

void Epsm::MixAudio(int16_t* out, uint32_t sampleCount, uint32_t sampleRate)
{
	_resampler.SetVolume(_console->GetNesConfig().EpsmVolume / 100.0);
	_resampler.SetSampleRates(_opn.GetSampleRate(), sampleRate);
	_resampler.Resample<true>(_samples.data(), (uint32_t)_samples.size() / 2, out, sampleCount, true);
	_samples.clear();
}

void Epsm::GetMemoryRanges(MemoryRanges& ranges)
{
	ranges.AddHandler(MemoryOperation::Write, 0x401C, 0x401F);
}

uint8_t Epsm::ReadRam(uint16_t addr)
{
	return 0;
}

void Epsm::Serialize(Serializer& s)
{
	SV(_clockCounter);
	SV(_prevOutPins);
	SV(_addr);
	SV(_data);
	SV(_masterClockRate);
	SV(_sampleClockCounter);

	SV(_opn);
}