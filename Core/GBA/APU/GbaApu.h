#pragma once
#include "pch.h"
#include "GBA/GbaTypes.h"
#include "GBA/APU/GbaApuFifo.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Audio/OnePoleLowPassFilter.h"

class Emulator;
class GbaConsole;
class GbaDmaController;
class GbaMemoryManager;

class GbaSquareChannel;
class GbaNoiseChannel;
class GbaWaveChannel;

class EmuSettings;
class SoundMixer;

class GbaApu final : public ISerializable
{
	static constexpr int MaxSampleRate = 256*1024;
	static constexpr int MaxSamples = MaxSampleRate * 8 / 60;

private:
	Emulator* _emu = nullptr;
	GbaConsole* _console = nullptr;
	GbaDmaController* _dmaController = nullptr;
	GbaMemoryManager* _memoryManager = nullptr;
	EmuSettings* _settings = nullptr;
	SoundMixer* _soundMixer = nullptr;
	
	unique_ptr<GbaSquareChannel> _square1;
	unique_ptr<GbaSquareChannel> _square2;
	unique_ptr<GbaWaveChannel> _wave;
	unique_ptr<GbaNoiseChannel> _noise;

	OnePoleLowPassFilter _filterL;
	OnePoleLowPassFilter _filterR;

	GbaApuState _state = {};
	GbaApuFifo _fifo[2] = {};

	int16_t* _soundBuffer = nullptr;
	uint32_t _sampleCount = 0;
	int16_t _rightSample = 0;
	int16_t _leftSample = 0;
	uint32_t _sampleRate = 32*1024;

	uint64_t _powerOnCycle = 0;
	uint64_t _prevClockCount = 0;
	uint8_t _enabledChannels = 0;

	typedef void(GbaApu::* Func)();
	Func _runFunc[16] = {};

	void ClockFrameSequencer();
	void UpdateSampleRate();

	template<bool sq1Enabled, bool sq2Enabled, bool waveEnabled, bool noiseEnabled>
	void InternalRun();

public:
	GbaApu();
	~GbaApu();
	
	void Init(Emulator* emu, GbaConsole* console, GbaDmaController* dmaController, GbaMemoryManager* memoryManager);

	GbaApuDebugState GetState();

	__forceinline void Run()
	{
		(this->*_runFunc[_enabledChannels])();
	}

	void PlayQueuedAudio();

	uint8_t ReadRegister(uint32_t addr);
	void WriteRegister(GbaAccessModeVal mode, uint32_t addr, uint8_t value);

	void ClockFifo(uint8_t timerIndex);

	void UpdateEnabledChannels();

	bool IsOddApuCycle();
	uint64_t GetElapsedApuCycles();

	void Serialize(Serializer& s) override;
};