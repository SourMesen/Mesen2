#pragma once
#include "stdafx.h"
#include "Breakpoint.h"

class ExpressionEvaluator;
class Debugger;
struct MemoryOperationInfo;
struct ExpressionData;
struct AddressInfo;
enum class MemoryOperationType;

class BreakpointManager
{
private:
	static constexpr int BreakpointTypeCount = 3; //Read, Write, Exec
	static constexpr int CategoryCount = 5; //CPU, VRAM, OAM, CGRAM, SPC

	Debugger *_debugger;
	
	vector<Breakpoint> _breakpoints[CategoryCount][BreakpointTypeCount];
	vector<ExpressionData> _rpnList[CategoryCount][BreakpointTypeCount];
	bool _hasBreakpoint[CategoryCount][BreakpointTypeCount] = {};

	unique_ptr<ExpressionEvaluator> _bpExpEval;

	BreakpointType GetBreakpointType(MemoryOperationType type);

public:
	BreakpointManager(Debugger *debugger);

	void SetBreakpoints(Breakpoint breakpoints[], uint32_t count);
	bool CheckBreakpoint(MemoryOperationInfo operationInfo, AddressInfo &address);
};