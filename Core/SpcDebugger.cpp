#include "stdafx.h"
#include "SpcDebugger.h"
#include "DisassemblyInfo.h"
#include "Disassembler.h"
#include "Spc.h"
#include "TraceLogger.h"
#include "CallstackManager.h"
#include "MemoryManager.h"
#include "Debugger.h"
#include "Console.h"
#include "MemoryAccessCounter.h"

SpcDebugger::SpcDebugger(Debugger* debugger)
{
	_debugger = debugger;
	_traceLogger = debugger->GetTraceLogger().get();
	_disassembler = debugger->GetDisassembler().get();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter().get();
	_spc = debugger->GetConsole()->GetSpc().get();
	_memoryManager = debugger->GetConsole()->GetMemoryManager().get();

	_callstackManager.reset(new CallstackManager(debugger));
	_step.reset(new StepRequest());
}

void SpcDebugger::Reset()
{
	_prevOpCode = 0xFF;
}

void SpcDebugger::ProcessRead(uint16_t addr, uint8_t value, MemoryOperationType type)
{
	if(type == MemoryOperationType::DummyRead) {
		//Ignore all dummy reads for now
		return;
	}

	AddressInfo addressInfo = _spc->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation { addr, value, type };

	if(type == MemoryOperationType::ExecOpCode) {
		_disassembler->BuildCache(addressInfo, 0, CpuType::Spc);

		DebugState debugState;
		_debugger->GetState(debugState, true);

		DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo);
		_traceLogger->Log(CpuType::Spc, debugState, disInfo);

		if(_prevOpCode == 0x3F || _prevOpCode == 0x0F) {
			//JSR, BRK
			uint8_t opSize = DisassemblyInfo::GetOpSize(_prevOpCode, 0, CpuType::Spc);
			uint16_t returnPc = _prevProgramCounter + opSize;
			_callstackManager->Push(_prevProgramCounter, debugState.Spc.PC, returnPc, StackFrameFlags::None);
		} else if(_prevOpCode == 0x6F || _prevOpCode == 0x7F) {
			//RTS, RTI
			_callstackManager->Pop(debugState.Spc.PC);
		}

		if(_step->BreakAddress == (int32_t)debugState.Spc.PC && (_prevOpCode == 0x6F || _prevOpCode == 0x7F)) {
			//RTS/RTI found, if we're on the expected return address, break immediately (for step over/step out)
			_step->StepCount = 0;
		}

		_prevOpCode = value;
		_prevProgramCounter = debugState.Spc.PC;

		if(_step->StepCount > 0) {
			_step->StepCount--;
		}
	}

	_debugger->ProcessBreakConditions(_step->StepCount == 0, CpuType::Spc, operation, addressInfo);

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, type, _memoryManager->GetMasterClock());
}

void SpcDebugger::ProcessWrite(uint16_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo { addr, SnesMemoryType::SpcRam }; //Writes never affect the SPC ROM
	MemoryOperationInfo operation { addr, value, type };
	_debugger->ProcessBreakConditions(false, CpuType::Spc, operation, addressInfo);

	_disassembler->InvalidateCache(addressInfo, CpuType::Spc);

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, type, _memoryManager->GetMasterClock());
}

void SpcDebugger::Run()
{
	_step.reset(new StepRequest());
}

void SpcDebugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step;

	switch(type) {
		case StepType::Step: step.StepCount = stepCount; break;
		case StepType::StepOut: step.BreakAddress = _callstackManager->GetReturnAddress(); break;
		case StepType::StepOver:
			if(_prevOpCode == 0x3F || _prevOpCode == 0x0F) {
				//JSR, BRK
				step.BreakAddress = _prevProgramCounter + DisassemblyInfo::GetOpSize(_prevOpCode, 0, CpuType::Spc);
			} else {
				//For any other instruction, step over is the same as step into
				step.StepCount = 1;
			}
			break;
		
		case StepType::SpecificScanline:
		case StepType::PpuStep:
			break;
	}

	_step.reset(new StepRequest(step));
}

shared_ptr<CallstackManager> SpcDebugger::GetCallstackManager()
{
	return _callstackManager;
}