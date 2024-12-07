#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/IDebugger.h"

class Disassembler;
class Debugger;
class Cx4TraceLogger;
class SnesCodeDataLogger;
class Cx4;
class CallstackManager;
class MemoryAccessCounter;
class SnesMemoryManager;
class BreakpointManager;
class EmuSettings;

enum class MemoryOperationType;

class Cx4Debugger final : public IDebugger
{
	Debugger* _debugger;
	Disassembler* _disassembler;
	SnesCodeDataLogger* _codeDataLogger;
	MemoryAccessCounter* _memoryAccessCounter;
	SnesMemoryManager* _memoryManager;
	Cx4* _cx4;
	EmuSettings* _settings;

	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<CallstackManager> _callstackManager;
	unique_ptr<Cx4TraceLogger> _traceLogger;

	uint32_t _prevProgramCounter = 0;
	uint8_t _prevStackPointer = 0;
	uint8_t _prevOpCode = 0;

public:
	Cx4Debugger(Debugger* debugger);

	void Reset() override;

	void ProcessInstruction();
	void ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type);

	void Run() override;
	void Step(int32_t stepCount, StepType type) override;

	void SetProgramCounter(uint32_t addr, bool updateDebuggerOnly = false) override;
	uint32_t GetProgramCounter(bool getInstPc) override;
	uint64_t GetCpuCycleCount(bool forProfiler) override;
	DebuggerFeatures GetSupportedFeatures() override;

	BreakpointManager* GetBreakpointManager() override;
	CallstackManager* GetCallstackManager() override;
	IAssembler* GetAssembler() override;
	BaseEventManager* GetEventManager() override;
	ITraceLogger* GetTraceLogger() override;

	BaseState& GetState() override;
};