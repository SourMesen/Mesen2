#pragma once
#include "stdafx.h"
#include "DebugTypes.h"
#include "IDebugger.h"
#include "CpuTypes.h"
#include "DebugState.h"

class Disassembler;
class Debugger;
class TraceLogger;
class Cpu;
class CallstackManager;
class MemoryAccessCounter;
class MemoryManager;
class CodeDataLogger;
class EmuSettings;
class ScriptManager;
class EventManager;
class MemoryMappings;
class BreakpointManager;
class Sa1;
class Assembler;
enum class MemoryOperationType;

class CpuDebugger final : public IDebugger
{
	Debugger* _debugger;
	Disassembler* _disassembler;
	TraceLogger* _traceLogger;
	MemoryAccessCounter* _memoryAccessCounter;
	MemoryManager* _memoryManager;
	EmuSettings* _settings;
	CodeDataLogger* _codeDataLogger;
	Cpu* _cpu;
	Sa1* _sa1;

	shared_ptr<EventManager> _eventManager;
	shared_ptr<Assembler> _assembler;
	shared_ptr<CallstackManager> _callstackManager;
	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<StepRequest> _step;

	CpuType _cpuType;
	bool _enableBreakOnUninitRead = false;
	uint8_t _prevOpCode = 0xFF;
	uint32_t _prevProgramCounter = 0;
	DebugState _debugState;

	MemoryMappings& GetMemoryMappings();
	CpuState GetState();
	bool IsRegister(uint32_t addr);

public:
	CpuDebugger(Debugger* debugger, CpuType cpuType);

	void Reset();
	void ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type);
	void Run();
	void Step(int32_t stepCount, StepType type);
	void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi);
	void ProcessPpuCycle(uint16_t scanline, uint16_t cycle);

	shared_ptr<EventManager> GetEventManager();
	shared_ptr<Assembler> GetAssembler();
	shared_ptr<CallstackManager> GetCallstackManager();
	BreakpointManager* GetBreakpointManager();
};