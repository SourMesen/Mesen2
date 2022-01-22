#include "stdafx.h"
#include "SNES/Spc.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/SnesConsole.h"
#include "SNES/Debugger/SpcDebugger.h"
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

void SpcDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	if(type == MemoryOperationType::DummyRead) {
		//Ignore all dummy reads for now
		return;
	}

	AddressInfo addressInfo = _spc->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, SnesMemoryType::SpcMemory);
	BreakSource breakSource = BreakSource::Unspecified;

	if(type == MemoryOperationType::ExecOpCode) {
		SpcState& spcState = _spc->GetState();

		if(_traceLogger->IsEnabled() || _settings->CheckDebuggerFlag(DebuggerFlags::SpcDebuggerEnabled)) {
			_disassembler->BuildCache(addressInfo, 0, CpuType::Spc);

			if(_traceLogger->IsEnabled()) {
				DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::Spc);
				_traceLogger->Log(spcState, disInfo, operation);
			}
		}

		if(_prevOpCode == 0x3F || _prevOpCode == 0x0F) {
			//JSR, BRK
			uint8_t opSize = DisassemblyInfo::GetOpSize(_prevOpCode, 0, CpuType::Spc);
			uint16_t returnPc = _prevProgramCounter + opSize;
			AddressInfo src = _spc->GetAbsoluteAddress(_prevProgramCounter);
			AddressInfo ret = _spc->GetAbsoluteAddress(returnPc);
			_callstackManager->Push(src, _prevProgramCounter, addressInfo, spcState.PC, ret, returnPc, StackFrameFlags::None);
		} else if(_prevOpCode == 0x6F || _prevOpCode == 0x7F) {
			//RTS, RTI
			_callstackManager->Pop(addressInfo, spcState.PC);
		}

		if(_step->BreakAddress == (int32_t)spcState.PC && (_prevOpCode == 0x6F || _prevOpCode == 0x7F)) {
			//RTS/RTI found, if we're on the expected return address, break immediately (for step over/step out)
			_step->StepCount = 0;
		}

		_prevOpCode = value;
		_prevProgramCounter = spcState.PC;

		_step->ProcessCpuExec(&breakSource);

		if(_settings->CheckDebuggerFlag(DebuggerFlags::SpcDebuggerEnabled)) {
			//Break on BRK/STP
			if(value == 0x0F && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnBrk)) {
				breakSource = BreakSource::BreakOnBrk;
				_step->StepCount = 0;
			} else if(value == 0xFF && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnStp)) {
				breakSource = BreakSource::BreakOnStp;
				_step->StepCount = 0;
			}
		}
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
	} else if(type == MemoryOperationType::ExecOperand) {
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}
	} else {
		_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}
	}

	_debugger->ProcessBreakConditions(_step->StepCount == 0, GetBreakpointManager(), operation, addressInfo, breakSource);
}

void SpcDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo { (int32_t)addr, SnesMemoryType::SpcRam }; //Writes never affect the SPC ROM
	MemoryOperationInfo operation(addr, value, type, SnesMemoryType::SpcMemory);
	_debugger->ProcessBreakConditions(false, GetBreakpointManager(), operation, addressInfo);

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
