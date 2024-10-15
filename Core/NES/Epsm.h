#pragma once
#include "NES/OpnInterface.h"
#include "Shared/Audio/SoundMixer.h"
#include "Shared/Interfaces/IAudioProvider.h"
#include "Utilities/Audio/HermiteResampler.h"
#include "Utilities/ISerializable.h"

class Emulator;
class NesConsole;

class Epsm : public IAudioProvider, public INesMemoryHandler, public ISerializable
{
private:
	Emulator* _emu = nullptr;
	NesConsole* _console = nullptr;

	OpnInterface _opn;

	vector<int16_t> _samples;
	HermiteResampler _resampler;

	uint64_t _masterClockRate = 0;
	uint64_t _clockCounter = 0;
	uint8_t _sampleClockCounter = 0;
	uint8_t _prevOutPins = 0;
	uint8_t _data = 0;
	uint8_t _addr = 0;

	uint64_t GetTargetClock();

public:
	Epsm(Emulator* emu, NesConsole* console, vector<uint8_t>& adpcmRom);
	~Epsm();

	void Write(uint8_t dataBus, uint8_t outPins);
	void WriteRam(uint16_t addr, uint8_t value) override;
	void Exec();

	void OnRegionChanged();

	void MixAudio(int16_t* out, uint32_t sampleCount, uint32_t sampleRate) override;

	void GetMemoryRanges(MemoryRanges& ranges) override;
	uint8_t ReadRam(uint16_t addr) override;

	void Serialize(Serializer& s) override;
};