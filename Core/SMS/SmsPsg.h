#pragma once
#include "pch.h"
#include "SMS/SmsConsole.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Audio/SoundMixer.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Audio/blip_buf.h"

class SmsPsg final : public ISerializable
{
private:
	static constexpr int SampleRate = 96000;
	static constexpr int MaxSamples = 4000;
	static constexpr int16_t _volumeLut[16] = { 8192, 6507, 5168, 4105, 3261, 2590, 2058, 1642, 1298, 1031, 819, 651, 517, 410, 326, 0 };

	int16_t* _soundBuffer = nullptr;
	blip_t* _leftChannel = nullptr;
	blip_t* _rightChannel = nullptr;

	SoundMixer* _soundMixer = nullptr;
	EmuSettings* _settings = nullptr;
	SmsConsole* _console = nullptr;

	SmsPsgState _state = {};
	uint64_t _masterClock = 0;
	uint64_t _clockCounter = 0;
	int16_t _prevOutputLeft = 0;
	int16_t _prevOutputRight = 0;

	void RunNoise(SmsNoiseChannelState& noise);

public:
	SmsPsg(Emulator* emu, SmsConsole* console);

	SmsPsgState& GetState() { return _state; }

	void SetRegion(ConsoleRegion region);

	void Run();
	void PlayQueuedAudio();

	void Write(uint8_t value);
	void WritePanningReg(uint8_t value);

	void Serialize(Serializer& s) override;
};