#include "stdafx.h"
#include "Debugger.h"
#include "Cpu.h"
#include "Ppu.h"
#include "CpuTypes.h"
#include "DisassemblyInfo.h"
#include "TraceLogger.h"
#include "../Utilities/HexUtilities.h"

Debugger::Debugger(shared_ptr<Cpu> cpu, shared_ptr<Ppu> ppu, shared_ptr<MemoryManager> memoryManager)
{
	_cpu = cpu;
	_ppu = ppu;
	_memoryManager = memoryManager;
	_traceLogger.reset(new TraceLogger(this, memoryManager));

	_cpuStepCount = 0;
}

Debugger::~Debugger()
{
}

void Debugger::ProcessCpuRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	if(type == MemoryOperationType::ExecOpCode) {
		CpuState state = _cpu->GetState();
		DisassemblyInfo disassemblyInfo(state, _memoryManager.get());
		DebugState debugState;
		GetState(&debugState);
		_traceLogger->LogEffectiveAddress(_cpu->GetLastOperand());
		_traceLogger->Log(debugState, disassemblyInfo);

		if(_cpuStepCount > 0) {
			_cpuStepCount--;
			while(_cpuStepCount == 0) {
				std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(10));
			}
		}
	}
}

void Debugger::ProcessCpuWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	if(type == MemoryOperationType::ExecOpCode) {
		//_traceLogger->Trace
	}
}

void Debugger::Run()
{
	_cpuStepCount = -1;
}

void Debugger::Step(int32_t stepCount)
{
	_cpuStepCount = stepCount;
}

bool Debugger::IsExecutionStopped()
{
	//TODO
	return false;
}

void Debugger::GetState(DebugState *state)
{
	state->Cpu = _cpu->GetState();
	state->Ppu = _ppu->GetState();
}

shared_ptr<TraceLogger> Debugger::GetTraceLogger()
{
	return _traceLogger;
}