#pragma once
#include "stdafx.h"
#include "CpuTypes.h"
#include "PpuTypes.h"

class Console;
class Cpu;
class Ppu;
class BaseCartridge;
class MemoryManager;

enum class MemoryOperationType;
class TraceLogger;
class MemoryDumper;
//class Disassembler;

struct DebugState
{
	CpuState Cpu;
	PpuState Ppu;
	//ApuState apuState;
};

class Debugger
{
private:
	shared_ptr<Console> _console;
	shared_ptr<Cpu> _cpu;
	shared_ptr<Ppu> _ppu;
	shared_ptr<MemoryManager> _memoryManager;
	shared_ptr<BaseCartridge> _baseCartridge;

	shared_ptr<TraceLogger> _traceLogger;
	shared_ptr<MemoryDumper> _memoryDumper;
	//unique_ptr<Disassembler> _disassembler;

	atomic<int32_t> _cpuStepCount;

public:
	Debugger(shared_ptr<Console> console);
	~Debugger();

	void ProcessCpuRead(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessCpuWrite(uint32_t addr, uint8_t value, MemoryOperationType type);

	void Run();
	void Step(int32_t stepCount);
	bool IsExecutionStopped();

	void GetState(DebugState *state);

	shared_ptr<TraceLogger> GetTraceLogger();
	shared_ptr<MemoryDumper> GetMemoryDumper();
};