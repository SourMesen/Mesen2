#pragma once
#include "stdafx.h"
#include "DebugTypes.h"
#include "IDebugger.h"

class Disassembler;
class Debugger;
class TraceLogger;
class NecDsp;
class CallstackManager;
class MemoryAccessCounter;
class MemoryManager;
class BreakpointManager;
class EmuSettings;

enum class MemoryOperationType;

class NecDspDebugger final : public IDebugger
{
	Debugger* _debugger;
	Disassembler* _disassembler;
	TraceLogger* _traceLogger;
	NecDsp* _dsp;
	EmuSettings* _settings;

	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<StepRequest> _step;

	uint32_t _prevProgramCounter = 0;

public:
	NecDspDebugger(Debugger* debugger);

	void Reset();

	void ProcessRead(uint16_t addr, uint8_t value, MemoryOperationType type);
	void ProcessWrite(uint16_t addr, uint8_t value, MemoryOperationType type);
	void Run();
	void Step(int32_t stepCount, StepType type);
	shared_ptr<CallstackManager> GetCallstackManager();
	BreakpointManager* GetBreakpointManager();
};