#pragma once
#include "stdafx.h"
#include "Breakpoint.h"
#include "DebugTypes.h"

class ExpressionEvaluator;
class Debugger;
struct ExpressionData;
enum class MemoryOperationType;

class BreakpointManager
{
private:
	static constexpr int BreakpointTypeCount = 3; //Read, Write, Exec

	Debugger *_debugger;
	
	vector<Breakpoint> _breakpoints[(int)CpuType::Sa1 + 1][BreakpointTypeCount];
	vector<ExpressionData> _rpnList[(int)CpuType::Sa1 + 1][BreakpointTypeCount];
	bool _hasBreakpoint[(int)CpuType::Sa1 + 1][BreakpointTypeCount] = {};

	unique_ptr<ExpressionEvaluator> _bpExpEval[(int)CpuType::Sa1 + 1];

	BreakpointType GetBreakpointType(MemoryOperationType type);

public:
	BreakpointManager(Debugger *debugger);

	void SetBreakpoints(Breakpoint breakpoints[], uint32_t count);
	int CheckBreakpoint(CpuType cpuType, MemoryOperationInfo operationInfo, AddressInfo &address);
};