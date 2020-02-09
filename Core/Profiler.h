#pragma once
#include "stdafx.h"
#include <unordered_map>
#include <stack>
#include "DebugTypes.h"

class Debugger;
class MemoryManager;

struct ProfiledFunction
{
	uint64_t ExclusiveCycles = 0;
	uint64_t InclusiveCycles = 0;
	uint64_t CallCount = 0;
	uint64_t MinCycles = UINT64_MAX;
	uint64_t MaxCycles = 0;
	AddressInfo Address;
};

class Profiler
{
private:
	Debugger* _debugger;
	MemoryManager* _memoryManager;

	unordered_map<int32_t, ProfiledFunction> _functions;
	
	deque<int32_t> _functionStack;
	deque<StackFrameFlags> _stackFlags;
	deque<uint64_t> _cycleCountStack;

	uint64_t _currentCycleCount;
	uint64_t _prevMasterClock;
	int32_t _currentFunction;

	void InternalReset();
	void UpdateCycles();

public:
	Profiler(Debugger* debugger);
	~Profiler();

	void StackFunction(AddressInfo& addr, StackFrameFlags stackFlag);
	void UnstackFunction();

	void Reset();
	void GetProfilerData(ProfiledFunction* profilerData, uint32_t& functionCount);
};