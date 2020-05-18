#pragma once
#include "stdafx.h"
#include "GbSquareChannel.h"
#include "GbWaveChannel.h"
#include "GbNoiseChannel.h"
#include "../Utilities/blip_buf.h"
#include "../Utilities/ISerializable.h"

class Console;
class Gameboy;
class SoundMixer;

class GbApu : public ISerializable
{
private:
	static constexpr int ApuFrequency = 1024 * 1024 * 4; //4mhz
	static constexpr int MaxSamples = 4000;
	static constexpr int SampleRate = 96000;
	Console* _console = nullptr;
	Gameboy* _gameboy = nullptr;
	SoundMixer* _soundMixer = nullptr;

	GbSquareChannel _square1;
	GbSquareChannel _square2;
	GbWaveChannel _wave;
	GbNoiseChannel _noise;

	int16_t* _soundBuffer = nullptr;
	blip_t* _leftChannel = nullptr;
	blip_t* _rightChannel = nullptr;

	int16_t _prevLeftOutput = 0;
	int16_t _prevRightOutput = 0;
	uint32_t _clockCounter = 0;
	uint64_t _prevClockCount = 0;

	GbApuState _state = {};

public:
	GbApu(Console* console, Gameboy* gameboy);
	virtual ~GbApu();

	GbApuDebugState GetState();

	void Run();

	void ClockFrameSequencer();

	uint8_t Read(uint16_t addr);
	void Write(uint16_t addr, uint8_t value);

	void Serialize(Serializer& s) override;
};