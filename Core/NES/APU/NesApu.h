#pragma once

#include "pch.h"
#include "Utilities/ISerializable.h"
#include "NES/INesMemoryHandler.h"
#include "NES/NesTypes.h"

class NesConsole;
class SquareChannel;
class TriangleChannel;
class NoiseChannel;
class DeltaModulationChannel;
class ApuFrameCounter;
class NesSoundMixer;
class EmuSettings;

enum class FrameType;
enum class ConsoleRegion;

class NesApu : public ISerializable, public INesMemoryHandler
{
	friend ApuFrameCounter;

private:
	bool _apuEnabled;
	bool _needToRun;

	uint32_t _previousCycle;
	uint32_t _currentCycle;

	unique_ptr<SquareChannel> _square1;
	unique_ptr<SquareChannel> _square2;
	unique_ptr<TriangleChannel> _triangle;
	unique_ptr<NoiseChannel> _noise;
	unique_ptr<DeltaModulationChannel> _dmc;
	unique_ptr<ApuFrameCounter> _frameCounter;

	NesConsole* _console;
	NesSoundMixer* _mixer;
	EmuSettings* _settings;

	ConsoleRegion _region;

private:
	__forceinline bool NeedToRun(uint32_t currentCycle);

	void FrameCounterTick(FrameType type);
	uint8_t GetStatus();

public:
	NesApu(NesConsole* console);
	~NesApu();

	void Serialize(Serializer& s) override;

	void Reset(bool softReset);
	void SetRegion(ConsoleRegion region, bool forceInit = false);

	uint8_t ReadRam(uint16_t addr) override;
	uint8_t PeekRam(uint16_t addr) override;
	void WriteRam(uint16_t addr, uint8_t value) override;
	void GetMemoryRanges(MemoryRanges& ranges) override;

	ApuState GetState();

	void Exec();
	void ProcessCpuClock();
	void Run();
	void EndFrame();

	void AddExpansionAudioDelta(AudioChannel channel, int16_t delta);
	void SetApuStatus(bool enabled);
	bool IsApuEnabled();
	static ConsoleRegion GetApuRegion(NesConsole* console);
	uint16_t GetDmcReadAddress();
	void SetDmcReadBuffer(uint8_t value);
	void SetNeedToRun();
};