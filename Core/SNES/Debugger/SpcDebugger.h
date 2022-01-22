#pragma once
#include "stdafx.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/IDebugger.h"

class Disassembler;
class Debugger;
class SpcTraceLogger;
class Spc;
class CallstackManager;
class MemoryAccessCounter;
class MemoryManager;
class BreakpointManager;
class EmuSettings;

enum class MemoryOperationType;

class SpcDebugger final : public IDebugger
{
	Debugger* _debugger;
	Disassembler* _disassembler;
	MemoryAccessCounter* _memoryAccessCounter;
	MemoryManager* _memoryManager;
	Spc* _spc;
	EmuSettings* _settings;

	unique_ptr<CallstackManager> _callstackManager;
	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<SpcTraceLogger> _traceLogger;
	unique_ptr<StepRequest> _step;

	uint8_t _prevOpCode = 0xFF;
	uint32_t _prevProgramCounter = 0;
	
public:
	SpcDebugger(Debugger* debugger);

	void Reset() override;

	void ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type);
	
	void Run() override;
	void Step(int32_t stepCount, StepType type) override;

	CallstackManager* GetCallstackManager() override;
	BreakpointManager* GetBreakpointManager() override;
	IAssembler* GetAssembler() override;
	BaseEventManager* GetEventManager() override;
	CodeDataLogger* GetCodeDataLogger() override;
	ITraceLogger* GetTraceLogger() override;

	BaseState& GetState() override;
};