#pragma once
#include "stdafx.h"
#include "DebugTypes.h"
#include "IDebugger.h"

class Disassembler;
class Debugger;
class TraceLogger;
class CodeDataLogger;
class Gsu;
class MemoryAccessCounter;
class MemoryManager;
class BreakpointManager;
class EmuSettings;

enum class MemoryOperationType;

class GsuDebugger final : public IDebugger
{
	Debugger* _debugger;
	Disassembler* _disassembler;
	TraceLogger* _traceLogger;
	CodeDataLogger* _codeDataLogger;
	MemoryAccessCounter* _memoryAccessCounter;
	MemoryManager* _memoryManager;
	Gsu* _gsu;
	EmuSettings* _settings;

	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<StepRequest> _step;

	uint8_t _prevOpCode = 0xFF;
	uint32_t _prevProgramCounter = 0;

public:
	GsuDebugger(Debugger* debugger);

	void Reset() override;

	void ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type) override;
	void ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type) override;

	void Run() override;
	void Step(int32_t stepCount, StepType type) override;

	BreakpointManager* GetBreakpointManager() override;
	shared_ptr<CallstackManager> GetCallstackManager() override;
	shared_ptr<IAssembler> GetAssembler() override;
	shared_ptr<IEventManager> GetEventManager() override;
	shared_ptr<CodeDataLogger> GetCodeDataLogger() override;
};