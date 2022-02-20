#include "stdafx.h"
#include "SNES/Spc.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/SnesConsole.h"
#include "SNES/Debugger/SpcDebugger.h"
#include "SNES/Debugger/DummySpc.h"
#include "SNES/Debugger/TraceLogger/SpcTraceLogger.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/Debugger.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"
#include "MemoryOperationType.h"

SpcDebugger::SpcDebugger(Debugger* debugger)
{
	_debugger = debugger;
	_disassembler = debugger->GetDisassembler();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter();

	SnesConsole* console = (SnesConsole*)debugger->GetConsole();
	_spc = console->GetSpc();
	_memoryManager = console->GetMemoryManager();
	_settings = debugger->GetEmulator()->GetSettings();

	_dummyCpu.reset(new DummySpc(_spc->GetSpcRam()));

	_traceLogger.reset(new SpcTraceLogger(debugger, console->GetPpu(), console->GetMemoryManager()));

	_callstackManager.reset(new CallstackManager(debugger));
	_breakpointManager.reset(new BreakpointManager(debugger, CpuType::Spc, debugger->GetEventManager(CpuType::Snes)));
	_step.reset(new StepRequest());
}

void SpcDebugger::Reset()
{
	_callstackManager.reset(new CallstackManager(_debugger));
	_prevOpCode = 0xFF;
}

void SpcDebugger::ProcessConfigChange()
{
	_debuggerEnabled = _settings->CheckDebuggerFlag(DebuggerFlags::SpcDebuggerEnabled);
	_predictiveBreakpoints = _settings->CheckDebuggerFlag(DebuggerFlags::UsePredictiveBreakpoints);
}

void SpcDebugger::ProcessInstruction()
{
	SpcState& state = _spc->GetState();
	uint16_t addr = state.PC;
	uint8_t value = _spc->DebugRead(addr);
	AddressInfo addressInfo = _spc->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, MemoryOperationType::ExecOpCode, MemoryType::SpcMemory);

	_disassembler->BuildCache(addressInfo, 0, CpuType::Spc);

	if(_traceLogger->IsEnabled()) {
		DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::Spc);
		_traceLogger->Log(state, disInfo, operation);
	}

	if(_prevOpCode == 0x3F || _prevOpCode == 0x0F) {
		//JSR, BRK
		uint8_t opSize = DisassemblyInfo::GetOpSize(_prevOpCode, 0, CpuType::Spc);
		uint16_t returnPc = _prevProgramCounter + opSize;
		AddressInfo src = _spc->GetAbsoluteAddress(_prevProgramCounter);
		AddressInfo ret = _spc->GetAbsoluteAddress(returnPc);
		_callstackManager->Push(src, _prevProgramCounter, addressInfo, addr, ret, returnPc, StackFrameFlags::None);
	} else if(_prevOpCode == 0x6F || _prevOpCode == 0x7F) {
		//RTS, RTI
		_callstackManager->Pop(addressInfo, addr);

		if(_step->BreakAddress == (int32_t)addr) {
			//RTS/RTI - if we're on the expected return address, break immediately (for step over/step out)
			_step->Break(BreakSource::CpuStep);
		}
	}

	_prevOpCode = value;
	_prevProgramCounter = addr;

	_step->ProcessCpuExec();

	if(_debuggerEnabled) {
		//Break on BRK/STP
		if(value == 0x0F && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnBrk)) {
			_step->Break(BreakSource::BreakOnBrk);
		} else if(value == 0xFF && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnStp)) {
			_step->Break(BreakSource::BreakOnStp);
		}
	}

	if(_step->StepCount != 0 && _breakpointManager->HasBreakpoints() && _predictiveBreakpoints) {
		_dummyCpu->SetDummyState(state);
		_dummyCpu->Step();
		for(uint32_t i = 1; i < _dummyCpu->GetOperationCount(); i++) {
			MemoryOperationInfo memOp = _dummyCpu->GetOperationInfo(i);
			if(_breakpointManager->HasBreakpointForType(memOp.Type)) {
				AddressInfo absAddr = _spc->GetAbsoluteAddress(memOp.Address);
				_debugger->ProcessPredictiveBreakpoint(CpuType::Spc, _breakpointManager.get(), memOp, absAddr);
			}
		}
	}

	_debugger->ProcessBreakConditions(CpuType::Spc, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void SpcDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _spc->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::SpcMemory);

	if(type == MemoryOperationType::ExecOpCode) {
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
	} else if(type == MemoryOperationType::ExecOperand) {
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}
		_debugger->ProcessBreakConditions(CpuType::Spc, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	} else {
		_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}
		_debugger->ProcessBreakConditions(CpuType::Spc, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	}
}

void SpcDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo { (int32_t)addr, MemoryType::SpcRam }; //Writes never affect the SPC ROM
	MemoryOperationInfo operation(addr, value, type, MemoryType::SpcMemory);
	_debugger->ProcessBreakConditions(CpuType::Spc, *_step.get(), _breakpointManager.get(), operation, addressInfo);

	_disassembler->InvalidateCache(addressInfo, CpuType::Spc);

	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetMasterClock());
	
	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation);
	}
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

CallstackManager* SpcDebugger::GetCallstackManager()
{
	return _callstackManager.get();
}

BreakpointManager* SpcDebugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

IAssembler* SpcDebugger::GetAssembler()
{
	throw std::runtime_error("Assembler not supported for SPC");
}

BaseEventManager* SpcDebugger::GetEventManager()
{
	throw std::runtime_error("Event manager not supported for SPC");
}

CodeDataLogger* SpcDebugger::GetCodeDataLogger()
{
	return nullptr;
}

BaseState& SpcDebugger::GetState()
{
	return _spc->GetState();
}

ITraceLogger* SpcDebugger::GetTraceLogger()
{
	return _traceLogger.get();
}
