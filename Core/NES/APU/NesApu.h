#pragma once

#include "stdafx.h"
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
enum class NesModel;

class NesApu : public ISerializable, public INesMemoryHandler
{
	friend ApuFrameCounter;

	private:
		bool _apuEnabled;
		bool _needToRun;

		uint32_t _previousCycle;
		uint32_t _currentCycle;

		unique_ptr<SquareChannel> _squareChannel[2];
		unique_ptr<TriangleChannel> _triangleChannel;
		unique_ptr<NoiseChannel> _noiseChannel;
		unique_ptr<DeltaModulationChannel> _deltaModulationChannel;
		unique_ptr<ApuFrameCounter> _frameCounter;

		shared_ptr<NesConsole> _console;
		NesSoundMixer* _mixer;
		EmuSettings* _settings;

		NesModel _nesModel;

	private:
		__forceinline bool NeedToRun(uint32_t currentCycle);

		void FrameCounterTick(FrameType type);
		uint8_t GetStatus();

	public:
		NesApu(shared_ptr<NesConsole> console);
		~NesApu();

		void Serialize(Serializer& s) override;

		void Reset(bool softReset);
		void SetNesModel(NesModel model, bool forceInit = false);

		uint8_t ReadRam(uint16_t addr) override;
		uint8_t PeekRam(uint16_t addr) override;
		void WriteRam(uint16_t addr, uint8_t value) override;
		void GetMemoryRanges(MemoryRanges &ranges) override;

		ApuState GetState();

		void Exec();
		void ProcessCpuClock();
		void Run();
		void EndFrame();

		void AddExpansionAudioDelta(AudioChannel channel, int16_t delta);
		void SetApuStatus(bool enabled);
		bool IsApuEnabled();
		uint16_t GetDmcReadAddress();
		void SetDmcReadBuffer(uint8_t value);
		void SetNeedToRun();
};