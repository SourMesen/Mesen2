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
	
	vector<Breakpoint> _breakpoints[(int)DebugUtilities::GetLastCpuType() + 1][BreakpointTypeCount];
	vector<ExpressionData> _rpnList[(int)DebugUtilities::GetLastCpuType() + 1][BreakpointTypeCount];
	bool _hasBreakpoint[(int)DebugUtilities::GetLastCpuType() + 1][BreakpointTypeCount] = {};

	unique_ptr<ExpressionEvaluator> _bpExpEval[(int)DebugUtilities::GetLastCpuType() + 1];

	BreakpointType GetBreakpointType(MemoryOperationType type);

public:
	BreakpointManager(Debugger *debugger);

	void SetBreakpoints(Breakpoint breakpoints[], uint32_t count);
	int CheckBreakpoint(CpuType cpuType, MemoryOperationInfo operationInfo, AddressInfo &address);
};