#pragma once
#include "pch.h"
#include "Debugger/Breakpoint.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugUtilities.h"

class ExpressionEvaluator;
class Debugger;
class IDebugger;
class BaseEventManager;
struct ExpressionData;
enum class MemoryOperationType;

class BreakpointManager
{
private:
	static constexpr int BreakpointTypeCount = (int)MemoryOperationType::PpuRenderingRead + 1;

	Debugger* _debugger;
	IDebugger *_cpuDebugger;
	CpuType _cpuType;
	BaseEventManager *_eventManager;
	
	vector<Breakpoint> _breakpoints[BreakpointTypeCount];
	vector<ExpressionData> _rpnList[BreakpointTypeCount];
	bool _hasBreakpoint;
	bool _hasBreakpointType[BreakpointTypeCount] = {};

	unique_ptr<ExpressionEvaluator> _bpExpEval;

	BreakpointType GetBreakpointType(MemoryOperationType type);
	int InternalCheckBreakpoint(MemoryOperationInfo operationInfo, AddressInfo &address, bool processMarkedBreakpoints);

public:
	BreakpointManager(Debugger *debugger, IDebugger* cpuDebugger, CpuType cpuType, BaseEventManager* eventManager);

	void SetBreakpoints(Breakpoint breakpoints[], uint32_t count);
	
	__forceinline bool HasBreakpoints() { return _hasBreakpoint; }
	__forceinline bool HasBreakpointForType(MemoryOperationType opType);
	__forceinline int CheckBreakpoint(MemoryOperationInfo operationInfo, AddressInfo &address, bool processMarkedBreakpoints);
};

__forceinline bool BreakpointManager::HasBreakpointForType(MemoryOperationType opType)
{
	return _hasBreakpointType[(int)opType];
}

__forceinline int BreakpointManager::CheckBreakpoint(MemoryOperationInfo operationInfo, AddressInfo &address, bool processMarkedBreakpoints)
{
	if(!_hasBreakpointType[(int)operationInfo.Type]) {
		return -1;
	}
	return InternalCheckBreakpoint(operationInfo, address, processMarkedBreakpoints);
}