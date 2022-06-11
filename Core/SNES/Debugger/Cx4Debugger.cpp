#include "stdafx.h"
#include "Cx4Debugger.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/Debugger.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/ExpressionEvaluator.h"
#include "SNES/BaseCartridge.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/SnesConsole.h"
#include "SNES/Debugger/SnesCodeDataLogger.h"
#include "SNES/Debugger/TraceLogger/Cx4TraceLogger.h"
#include "SNES/Coprocessors/CX4/Cx4.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "SNES/MemoryMappings.h"
#include "MemoryOperationType.h"

Cx4Debugger::Cx4Debugger(Debugger* debugger)
{
	SnesConsole* console = (SnesConsole*)debugger->GetConsole();

	_debugger = debugger;
	_codeDataLogger = (SnesCodeDataLogger*)debugger->GetCodeDataLogger(CpuType::Snes);
	_disassembler = debugger->GetDisassembler();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter();
	_cx4 = console->GetCartridge()->GetCx4();
	_memoryManager = console->GetMemoryManager();
	_settings = debugger->GetEmulator()->GetSettings();
	
	_traceLogger.reset(new Cx4TraceLogger(debugger, this, console->GetPpu(), _memoryManager));

	_breakpointManager.reset(new BreakpointManager(debugger, this, CpuType::Cx4, debugger->GetEventManager(CpuType::Snes)));
	_step.reset(new StepRequest());
}

void Cx4Debugger::Reset()
{
}

void Cx4Debugger::ProcessInstruction()
{
	Cx4State& state = _cx4->GetState();
	uint32_t addr = (state.Cache.Address[state.Cache.Page] + (state.PC * 2)) & 0xFFFFFF;
	uint16_t value = _cx4->GetMemoryMappings()->PeekWord(addr);
	AddressInfo addressInfo = _cx4->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, MemoryOperationType::ExecOpCode, MemoryType::Cx4Memory);

	if(addressInfo.Type == MemoryType::SnesPrgRom) {
		_codeDataLogger->SetSnesCode<SnesCdlFlags::Cx4>(addressInfo.Address);
		_codeDataLogger->SetSnesCode<SnesCdlFlags::Cx4>(addressInfo.Address + 1);
	}

	if(_settings->CheckDebuggerFlag(DebuggerFlags::Cx4DebuggerEnabled)) {
		_disassembler->BuildCache(addressInfo, 0, CpuType::Cx4);
	}

	_prevProgramCounter = addr;

	_step->ProcessCpuExec();
	_debugger->ProcessBreakConditions(CpuType::Cx4, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void Cx4Debugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	Cx4State& state = _cx4->GetState();
	addr = (state.Cache.Address[state.Cache.Page] + (state.PC * 2)) & 0xFFFFFF;

	AddressInfo addressInfo = _cx4->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::Cx4Memory);

	if(type == MemoryOperationType::ExecOpCode) {
		if(_traceLogger->IsEnabled()) {
			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::Cx4);
			_traceLogger->Log(state, disInfo, operation);
		}

		AddressInfo opCodeHighAddr = _cx4->GetMemoryMappings()->GetAbsoluteAddress(addr + 1);
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
		_memoryAccessCounter->ProcessMemoryExec(opCodeHighAddr, _memoryManager->GetMasterClock());
	} else {
		if(addressInfo.Type == MemoryType::SnesPrgRom) {
			_codeDataLogger->SetData<SnesCdlFlags::Cx4>(addressInfo.Address);
		}
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}
		_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());

		_debugger->ProcessBreakConditions(CpuType::Cx4, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	}
}

void Cx4Debugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _cx4->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::Cx4Memory);
	_debugger->ProcessBreakConditions(CpuType::Cx4, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetMasterClock());
	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation);
	}
}

void Cx4Debugger::Run()
{
	_step.reset(new StepRequest());
}

void Cx4Debugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step;

	switch(type) {
		case StepType::Step: step.StepCount = stepCount; break;

		case StepType::StepOut:
		case StepType::StepOver:
			step.StepCount = 1;
			break;

		case StepType::SpecificScanline:
		case StepType::PpuStep:
			break;
	}

	_step.reset(new StepRequest(step));
}

void Cx4Debugger::SetProgramCounter(uint32_t addr)
{
	//Not implemented
}

uint32_t Cx4Debugger::GetProgramCounter(bool getInstPc)
{
	Cx4State& state = _cx4->GetState();
	return getInstPc ? _prevProgramCounter : ((state.Cache.Address[state.Cache.Page] + (state.PC * 2)) & 0xFFFFFF);
}

BreakpointManager* Cx4Debugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

CallstackManager* Cx4Debugger::GetCallstackManager()
{
	return nullptr;
}

IAssembler* Cx4Debugger::GetAssembler()
{
	throw std::runtime_error("Assembler not supported for CX4");
}

BaseEventManager* Cx4Debugger::GetEventManager()
{
	throw std::runtime_error("Event manager not supported for CX4");
}

CodeDataLogger* Cx4Debugger::GetCodeDataLogger()
{
	return nullptr;
}

BaseState& Cx4Debugger::GetState()
{
	return _cx4->GetState();
}

ITraceLogger* Cx4Debugger::GetTraceLogger()
{
	return _traceLogger.get();
}
