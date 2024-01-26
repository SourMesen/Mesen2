#pragma once
#include "pch.h"
#include "Shared/RewindManager.h"

class Emulator;
class IDebugger;

struct StepBackCacheEntry
{
	stringstream SaveState;
	uint64_t Clock;
};

struct StepBackConfig
{
	uint64_t CurrentCycle;
	uint32_t CyclesPerScanline;
	uint32_t CyclesPerFrame;
};

enum class StepBackType
{
	Instruction,
	Scanline,
	Frame
};

class StepBackManager
{
private:
	static constexpr uint64_t DefaultClockLimit = 600; //Default to 600 clocks to avoid retry when NES sprite DMA occurs (~512 cycles)

	Emulator* _emu = nullptr;
	RewindManager* _rewindManager = nullptr;
	IDebugger* _debugger = nullptr;

	vector<StepBackCacheEntry> _cache;
	uint64_t _targetClock = 0;
	uint64_t _prevClock = 0;
	bool _active = false;
	bool _allowRetry = false;
	uint64_t _stateClockLimit = StepBackManager::DefaultClockLimit;

public:
	StepBackManager(Emulator* emu, IDebugger* debugger);

	void StepBack(StepBackType type);
	bool CheckStepBack();

	void ResetCache() { _cache.clear(); }
	bool IsRewinding() { return _active || _rewindManager->IsRewinding(); }
};