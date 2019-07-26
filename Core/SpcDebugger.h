#pragma once
#include "stdafx.h"
#include "DebugTypes.h"

class Disassembler;
class Debugger;
class TraceLogger;
class Spc;
class CallstackManager;
class MemoryAccessCounter;
class MemoryManager;

class SpcDebugger
{
	Debugger* _debugger;
	Disassembler* _disassembler;
	TraceLogger* _traceLogger;
	MemoryAccessCounter* _memoryAccessCounter;
	MemoryManager* _memoryManager;
	Spc* _spc;
	
	shared_ptr<CallstackManager> _callstackManager;
	unique_ptr<StepRequest> _step;

	uint8_t _prevOpCode = 0xFF;
	uint32_t _prevProgramCounter = 0;

public:
	SpcDebugger(Debugger* debugger);

	void Reset();

	void ProcessRead(uint16_t addr, uint8_t value, MemoryOperationType type);
	void ProcessWrite(uint16_t addr, uint8_t value, MemoryOperationType type);
	void Run();
	void Step(int32_t stepCount, StepType type);
	shared_ptr<CallstackManager> GetCallstackManager();
};