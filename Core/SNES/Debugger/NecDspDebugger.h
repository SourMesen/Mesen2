#pragma once
#include "stdafx.h"
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

	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<StepRequest> _step;
	unique_ptr<NecDspTraceLogger> _traceLogger;

	uint32_t _prevProgramCounter = 0;

public:
	NecDspDebugger(Debugger* debugger);

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