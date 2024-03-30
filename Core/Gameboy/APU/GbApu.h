#pragma once
#include "pch.h"
#include "Gameboy/APU/GbSquareChannel.h"
#include "Gameboy/APU/GbWaveChannel.h"
#include "Gameboy/APU/GbNoiseChannel.h"
#include "Utilities/Audio/blip_buf.h"
#include "Utilities/ISerializable.h"

class Emulator;
class Gameboy;
class SoundMixer;
class EmuSettings;

class GbApu : public ISerializable
{
public:
	static constexpr int SampleRate = 96000;

private:
	static constexpr int ApuFrequency = 1024 * 1024 * 4; //4mhz
	static constexpr int MaxSamples = 4000;
	Emulator* _emu = nullptr;
	Gameboy* _gameboy = nullptr;
	EmuSettings* _settings = nullptr;
	SoundMixer* _soundMixer = nullptr;

	unique_ptr<GbSquareChannel> _square1;
	unique_ptr<GbSquareChannel> _square2;
	unique_ptr<GbWaveChannel> _wave;
	unique_ptr<GbNoiseChannel> _noise;

	int16_t* _soundBuffer = nullptr;
	blip_t* _leftChannel = nullptr;
	blip_t* _rightChannel = nullptr;

	int16_t _prevLeftOutput = 0;
	int16_t _prevRightOutput = 0;
	uint32_t _clockCounter = 0;
	uint64_t _prevClockCount = 0;

	uint32_t _skipFirstEventCounter = 0;
	uint64_t _powerOnCycle = 0;

	GbApuState _state = {};

	uint8_t InternalRead(uint16_t addr);

public:
	GbApu();
	virtual ~GbApu();

	void Init(Emulator* emu, Gameboy* gameboy);

	GbApuDebugState GetState();

	bool IsOddApuCycle();
	uint64_t GetElapsedApuCycles();

	void Run();

	void PlayQueuedAudio();

	void GetSoundSamples(int16_t* &samples, uint32_t& sampleCount);

	void ClockFrameSequencer();

	uint8_t Peek(uint16_t addr);
	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	uint8_t ReadCgbRegister(uint16_t addr);

	template<typename T> void ProcessLengthEnableFlag(uint8_t value, T& length, bool& lengthEnabled, bool& enabled);

	void Serialize(Serializer& s) override;
};
