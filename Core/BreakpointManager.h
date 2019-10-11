#pragma once
#include "stdafx.h"
#include "Breakpoint.h"
#include "DebugTypes.h"
#include "DebugUtilities.h"

class ExpressionEvaluator;
class Debugger;
struct ExpressionData;
enum class MemoryOperationType;

class BreakpointManager
{
private:
	static constexpr int BreakpointTypeCount = 3; //Read, Write, Exec

	Debugger *_debugger;
	CpuType _cpuType;
	
	vector<Breakpoint> _breakpoints[BreakpointTypeCount];
	vector<ExpressionData> _rpnList[BreakpointTypeCount];
	bool _hasBreakpoint;
	bool _hasBreakpointType[BreakpointTypeCount] = {};

	unique_ptr<ExpressionEvaluator> _bpExpEval;

	BreakpointType GetBreakpointType(MemoryOperationType type);
	int InternalCheckBreakpoint(MemoryOperationInfo operationInfo, AddressInfo &address);

public:
	BreakpointManager(Debugger *debugger, CpuType cpuType);

	void SetBreakpoints(Breakpoint breakpoints[], uint32_t count);
	__forceinline int CheckBreakpoint(MemoryOperationInfo operationInfo, AddressInfo &address);
};

__forceinline int BreakpointManager::CheckBreakpoint(MemoryOperationInfo operationInfo, AddressInfo &address)
{
	if(!_hasBreakpoint) {
		return -1;
	}
	return InternalCheckBreakpoint(operationInfo, address);
}