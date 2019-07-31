#include "stdafx.h"
#include "GsuDebugger.h"
#include "DisassemblyInfo.h"
#include "Disassembler.h"
#include "BaseCartridge.h"
#include "Gsu.h"
#include "TraceLogger.h"
#include "CallstackManager.h"
#include "MemoryManager.h"
#include "Debugger.h"
#include "Console.h"
#include "MemoryAccessCounter.h"

GsuDebugger::GsuDebugger(Debugger* debugger)
{
	_debugger = debugger;
	_traceLogger = debugger->GetTraceLogger().get();
	_disassembler = debugger->GetDisassembler().get();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter().get();
	_gsu = debugger->GetConsole()->GetCartridge()->GetGsu();
	_memoryManager = debugger->GetConsole()->GetMemoryManager().get();

	_step.reset(new StepRequest());
}

void GsuDebugger::Reset()
{
	_prevOpCode = 0xFF;
}

void GsuDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	if(type == MemoryOperationType::DummyRead) {
		//Ignore all dummy reads for now
		return;
	}

	AddressInfo addressInfo = _gsu->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation { addr, value, type };

	if(type == MemoryOperationType::ExecOpCode) {
		DebugState debugState;
		_debugger->GetState(debugState, true);

		_disassembler->BuildCache(addressInfo, debugState.Gsu.SFR.GetFlagsHigh() & 0x13, CpuType::Gsu);
		debugState.Gsu.R[15] = addr;

		DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo);
		_traceLogger->Log(CpuType::Gsu, debugState, disInfo);
		
		_prevOpCode = value;
		_prevProgramCounter = addr;

		if(_step->StepCount > 0) {
			_step->StepCount--;
		}
	}

	_debugger->ProcessBreakConditions(_step->StepCount == 0, CpuType::Gsu, operation, addressInfo);

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, type, _memoryManager->GetMasterClock());
}

void GsuDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _gsu->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation { addr, value, type };
	_debugger->ProcessBreakConditions(false, CpuType::Gsu, operation, addressInfo);

	_disassembler->InvalidateCache(addressInfo, CpuType::Gsu);

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, type, _memoryManager->GetMasterClock());
}

void GsuDebugger::Run()
{
	_step.reset(new StepRequest());
}

void GsuDebugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step;

	switch(type) {
		case StepType::StepOut:
		case StepType::StepOver:
		case StepType::Step: step.StepCount = stepCount; break;
		
		case StepType::SpecificScanline:
		case StepType::PpuStep:
			break;
	}

	_step.reset(new StepRequest(step));
}
