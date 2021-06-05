#include "stdafx.h"
#include <limits>
#include "Profiler.h"
#include "DebugBreakHelper.h"
#include "Debugger.h"
#include "SNES/Console.h"
#include "MemoryDumper.h"
#include "DebugTypes.h"

static constexpr int32_t ResetFunctionIndex = -1;

Profiler::Profiler(Debugger* debugger)
{
	_debugger = debugger;
	_emu = debugger->GetEmulator();
	InternalReset();
}

Profiler::~Profiler()
{
}

void Profiler::StackFunction(AddressInfo &addr, StackFrameFlags stackFlag)
{
	if(addr.Address >= 0) {
		uint32_t key = addr.Address | ((uint8_t)addr.Type << 24);
		if(_functions.find(key) == _functions.end()) {
			_functions[key] = ProfiledFunction();
			_functions[key].Address = addr;
		}

		UpdateCycles();

		_stackFlags.push_back(stackFlag);
		_cycleCountStack.push_back(_currentCycleCount);
		_functionStack.push_back(_currentFunction);

		ProfiledFunction& func = _functions[key];
		func.CallCount++;

		_currentFunction = key;
		_currentCycleCount = 0;
	}
}

void Profiler::UpdateCycles()
{
	uint64_t masterClock = _emu->GetMasterClock();
	
	ProfiledFunction& func = _functions[_currentFunction];
	uint64_t clockGap = masterClock - _prevMasterClock;
	func.ExclusiveCycles += clockGap;
	func.InclusiveCycles += clockGap;
	
	int32_t len = (int32_t)_functionStack.size();
	for(int32_t i = len - 1; i >= 0; i--) {
		_functions[_functionStack[i]].InclusiveCycles += clockGap;
		if(_stackFlags[i] != StackFrameFlags::None) {
			//Don't apply inclusive times to stack frames before an IRQ/NMI
			break;
		}
	}

	_currentCycleCount += clockGap;
	_prevMasterClock = masterClock;
}

void Profiler::UnstackFunction()
{
	if(!_functionStack.empty()) {
		UpdateCycles();

		//Return to the previous function
		ProfiledFunction& func = _functions[_currentFunction];
		func.MinCycles = std::min(func.MinCycles, _currentCycleCount);
		func.MaxCycles = std::max(func.MaxCycles, _currentCycleCount);

		_currentFunction = _functionStack.back();
		_functionStack.pop_back();
		_stackFlags.pop_back();

		//Add the subroutine's cycle count to the current routine's cycle count
		_currentCycleCount = _cycleCountStack.back() + _currentCycleCount;
		_cycleCountStack.pop_back();
	}
}

void Profiler::Reset()
{
	DebugBreakHelper helper(_debugger);
	InternalReset();
}

void Profiler::InternalReset()
{
	_prevMasterClock = _emu->GetMasterClock();
	_currentCycleCount = 0;
	_currentFunction = ResetFunctionIndex;
	_functionStack.clear();
	_stackFlags.clear();
	_cycleCountStack.clear();
	
	_functions.clear();
	_functions[ResetFunctionIndex] = ProfiledFunction();
	_functions[ResetFunctionIndex].Address = { ResetFunctionIndex, SnesMemoryType::Register };
}

void Profiler::GetProfilerData(ProfiledFunction* profilerData, uint32_t& functionCount)
{
	DebugBreakHelper helper(_debugger);
	
	UpdateCycles();

	functionCount = 0;
	for(auto& func : _functions) {
		profilerData[functionCount] = func.second;
		functionCount++;

		if(functionCount >= 100000) {
			break;
		}
	}
}
