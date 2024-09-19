#pragma once
#include "pch.h"
#include "WS/WsTypes.h"
#include "Utilities/ISerializable.h"
#include "Utilities/Audio/OnePoleLowPassFilter.h"

class Emulator;
class WsConsole;
class WsMemoryManager;
class WsDmaController;
class WsApuCh1;
class WsApuCh2;
class WsApuCh3;
class WsApuCh4;
class WsHyperVoice;
class SoundMixer;

class WsApu final : public ISerializable
{
private:
	static constexpr int ApuFrequency = 24000;
	static constexpr int MaxSamples = 256;

private:
	WsApuState _state = {};

	Emulator* _emu = nullptr;
	WsConsole* _console = nullptr;
	WsMemoryManager* _memoryManager = nullptr;
	WsDmaController* _dmaController = nullptr;
	SoundMixer* _soundMixer = nullptr;

	unique_ptr<WsApuCh1> _ch1;
	unique_ptr<WsApuCh2> _ch2;
	unique_ptr<WsApuCh3> _ch3;
	unique_ptr<WsApuCh4> _ch4;
	unique_ptr<WsHyperVoice> _hyperVoice;

	OnePoleLowPassFilter _filterL;
	OnePoleLowPassFilter _filterR;

	int16_t* _soundBuffer = nullptr;

	uint32_t _clockCounter = 0;
	uint16_t _sampleCount = 0;

	void UpdateOutput();
	uint16_t GetApuOutput(bool forRight);

public:
	WsApu(Emulator* emu, WsConsole* console, WsMemoryManager* memoryManager, WsDmaController* dmaController);
	~WsApu();

	void ChangeMasterVolume();

	void PlayQueuedAudio();
	void WriteDma(bool forHyperVoice, uint8_t sampleValue);

	WsApuState& GetState() { return _state; }

	uint8_t GetMasterVolume() { return _state.InternalMasterVolume; }

	uint8_t ReadSample(uint8_t ch, uint8_t pos);
	void Run();

	uint8_t Read(uint16_t port);
	void Write(uint16_t port, uint8_t value);

	void Serialize(Serializer& s) override;
};