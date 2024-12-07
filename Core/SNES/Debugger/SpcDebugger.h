#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/IDebugger.h"

class Disassembler;
class Debugger;
class SpcTraceLogger;
class Spc;
class DummySpc;
class CallstackManager;
class MemoryAccessCounter;
class SnesMemoryManager;
class BreakpointManager;
class EmuSettings;

enum class MemoryOperationType;

class SpcDebugger final : public IDebugger
{
	Debugger* _debugger;
	Disassembler* _disassembler;
	MemoryAccessCounter* _memoryAccessCounter;
	SnesMemoryManager* _memoryManager;
	Spc* _spc;
	EmuSettings* _settings;

	unique_ptr<CallstackManager> _callstackManager;
	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<SpcTraceLogger> _traceLogger;
	unique_ptr<DummySpc> _dummyCpu;

	uint8_t _prevOpCode = 0xFF;
	uint8_t _prevStackPointer = 0;
	uint32_t _prevProgramCounter = 0;

	bool _debuggerEnabled = false;
	bool _predictiveBreakpoints = false;
	bool _ignoreDspReadWrites = false;
	
public:
	SpcDebugger(Debugger* debugger);

	void Reset() override;

	void ProcessConfigChange() override;

	void ProcessInstruction();

	template<MemoryAccessFlags flags>
	void ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type);
	
	template<MemoryAccessFlags flags>
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