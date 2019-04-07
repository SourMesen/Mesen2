#pragma once
#include "stdafx.h"
#include "CpuTypes.h"
#include "PpuTypes.h"
#include "DebugTypes.h"

class Console;
class Cpu;
class Ppu;
class Spc;
class BaseCartridge;
class MemoryManager;

class TraceLogger;
class ExpressionEvaluator;
class MemoryDumper;
class MemoryAccessCounter;
class Disassembler;
class BreakpointManager;
class PpuTools;
class CodeDataLogger;
class EventManager;
class CallstackManager;

enum class EventType;
enum class EvalResultType : int32_t;

class Debugger
{
private:
	shared_ptr<Console> _console;
	shared_ptr<Cpu> _cpu;
	shared_ptr<Ppu> _ppu;
	shared_ptr<Spc> _spc;
	shared_ptr<MemoryManager> _memoryManager;
	shared_ptr<BaseCartridge> _baseCartridge;

	shared_ptr<TraceLogger> _traceLogger;
	shared_ptr<MemoryDumper> _memoryDumper;
	shared_ptr<MemoryAccessCounter> _memoryAccessCounter;
	shared_ptr<CodeDataLogger> _codeDataLogger;
	shared_ptr<Disassembler> _disassembler;
	shared_ptr<BreakpointManager> _breakpointManager;
	shared_ptr<PpuTools> _ppuTools;
	shared_ptr<EventManager> _eventManager;
	shared_ptr<CallstackManager> _callstackManager;
	shared_ptr<CallstackManager> _spcCallstackManager;

	unique_ptr<ExpressionEvaluator> _watchExpEval;

	atomic<bool> _executionStopped;
	atomic<uint32_t> _breakRequestCount;

	atomic<int32_t> _cpuStepCount;
	atomic<int32_t> _spcStepCount;
	atomic<int32_t> _ppuStepCount;
	atomic<int32_t> _cpuBreakAddress;
	atomic<int32_t> _spcBreakAddress;
	atomic<int32_t> _breakScanline;
	
	uint8_t _prevOpCode = 0;
	uint32_t _prevProgramCounter = 0;

	uint8_t _spcPrevOpCode = 0;
	uint32_t _spcPrevProgramCounter = 0;

	void SleepUntilResume();
	void ProcessStepConditions(uint8_t opCode, uint32_t currentPc);
	void ProcessBreakConditions(MemoryOperationInfo &operation, AddressInfo &addressInfo);

public:
	Debugger(shared_ptr<Console> console);
	~Debugger();
	void Release();

	void ProcessCpuRead(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessCpuWrite(uint32_t addr, uint8_t value, MemoryOperationType type);

	void ProcessWorkRamRead(uint32_t addr, uint8_t value);
	void ProcessWorkRamWrite(uint32_t addr, uint8_t value);

	void ProcessSpcRead(uint16_t addr, uint8_t value, MemoryOperationType type);
	void ProcessSpcWrite(uint16_t addr, uint8_t value, MemoryOperationType type);

	void ProcessPpuRead(uint16_t addr, uint8_t value, SnesMemoryType memoryType);
	void ProcessPpuWrite(uint16_t addr, uint8_t value, SnesMemoryType memoryType);
	void ProcessPpuCycle();

	void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi);
	void ProcessEvent(EventType type);

	int32_t EvaluateExpression(string expression, EvalResultType &resultType, bool useCache);

	void Run();
	void Step(int32_t stepCount, StepType type = StepType::CpuStep);
	bool IsExecutionStopped();

	void BreakRequest(bool release);

	void GetState(DebugState &state);

	shared_ptr<TraceLogger> GetTraceLogger();
	shared_ptr<MemoryDumper> GetMemoryDumper();
	shared_ptr<MemoryAccessCounter> GetMemoryAccessCounter();
	shared_ptr<CodeDataLogger> GetCodeDataLogger();
	shared_ptr<Disassembler> GetDisassembler();
	shared_ptr<BreakpointManager> GetBreakpointManager();
	shared_ptr<PpuTools> GetPpuTools();
	shared_ptr<EventManager> GetEventManager();
	shared_ptr<CallstackManager> GetCallstackManager(CpuType cpuType);
	shared_ptr<Console> GetConsole();
};