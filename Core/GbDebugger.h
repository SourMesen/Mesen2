#pragma once
#include "stdafx.h"
#include "DebugTypes.h"
#include "IDebugger.h"

class Disassembler;
class Debugger;
class TraceLogger;
class Gameboy;
class CallstackManager;
class MemoryAccessCounter;
class MemoryManager;
class BreakpointManager;
class EmuSettings;
class GbEventManager;
class GbAssembler;

class GbDebugger final : public IDebugger
{
	Debugger* _debugger;
	Disassembler* _disassembler;
	TraceLogger* _traceLogger;
	MemoryAccessCounter* _memoryAccessCounter;
	MemoryManager* _memoryManager;
	Gameboy* _gameboy;
	EmuSettings* _settings;

	shared_ptr<GbEventManager> _eventManager;
	shared_ptr<CallstackManager> _callstackManager;
	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<StepRequest> _step;
	shared_ptr<GbAssembler> _assembler;

	uint8_t _prevOpCode = 0xFF;
	uint32_t _prevProgramCounter = 0;

public:
	GbDebugger(Debugger* debugger);
	~GbDebugger();

	void Reset();

	void ProcessRead(uint16_t addr, uint8_t value, MemoryOperationType type);
	void ProcessWrite(uint16_t addr, uint8_t value, MemoryOperationType type);
	void Run();
	void Step(int32_t stepCount, StepType type);
	void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc);

	shared_ptr<GbEventManager> GetEventManager();
	shared_ptr<GbAssembler> GetAssembler();
	shared_ptr<CallstackManager> GetCallstackManager();
	BreakpointManager* GetBreakpointManager();
};