#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/IDebugger.h"

class Disassembler;
class Debugger;
class NecDspTraceLogger;
class NecDsp;
class CallstackManager;
class MemoryAccessCounter;
class SnesMemoryManager;
class BreakpointManager;
class EmuSettings;

enum class MemoryOperationType;

class NecDspDebugger final : public IDebugger
{
	Debugger* _debugger;
	Disassembler* _disassembler;
	NecDsp* _dsp;
	EmuSettings* _settings;
	MemoryAccessCounter* _memoryAccessCounter;
	SnesMemoryManager* _memoryManager;

	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<NecDspTraceLogger> _traceLogger;
	unique_ptr<CallstackManager> _callstackManager;

	uint32_t _prevProgramCounter = 0;
	uint8_t _prevStackPointer = 0;
	uint32_t _prevOpCode = 0;

public:
	NecDspDebugger(Debugger* debugger);

	void Reset() override;

	void ProcessInstruction();
	void ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type);
	
	void Run() override;
	void Step(int32_t stepCount, StepType type) override;

	DebuggerFeatures GetSupportedFeatures() override;
	void SetProgramCounter(uint32_t addr, bool updateDebuggerOnly = false) override;
	uint32_t GetProgramCounter(bool getInstPc) override;
	uint64_t GetCpuCycleCount(bool forProfiler) override;
	CallstackManager* GetCallstackManager() override;
	BreakpointManager* GetBreakpointManager() override;
	IAssembler* GetAssembler() override;
	BaseEventManager* GetEventManager() override;
	ITraceLogger* GetTraceLogger() override;

	BaseState& GetState() override;
};