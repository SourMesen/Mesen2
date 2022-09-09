#pragma once
#include "pch.h"
#include "DebugTypes.h"

class Debugger;
class IConsole;

struct ProfiledFunction
{
	uint64_t ExclusiveCycles = 0;
	uint64_t InclusiveCycles = 0;
	uint64_t CallCount = 0;
	uint64_t MinCycles = UINT64_MAX;
	uint64_t MaxCycles = 0;
	AddressInfo Address = {};
};

class Profiler
{
private:
	Debugger* _debugger = nullptr;
	IConsole* _console = nullptr;

	unordered_map<int32_t, ProfiledFunction> _functions;
	
	deque<int32_t> _functionStack;
	deque<StackFrameFlags> _stackFlags;
	deque<uint64_t> _cycleCountStack;

	uint64_t _currentCycleCount = 0;
	uint64_t _prevMasterClock = 0;
	int32_t _currentFunction = -1;

	void InternalReset();
	void UpdateCycles();

public:
	Profiler(Debugger* debugger, IConsole* _console);
	~Profiler();

	void StackFunction(AddressInfo& addr, StackFrameFlags stackFlag);
	void UnstackFunction();

	void Reset();
	void ResetState();
	void GetProfilerData(ProfiledFunction* profilerData, uint32_t& functionCount);
};