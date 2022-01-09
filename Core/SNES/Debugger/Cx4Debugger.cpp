#include "stdafx.h"
#include "Cx4Debugger.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/Debugger.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Debugger/CodeDataLogger.h"
#include "SNES/BaseCartridge.h"
#include "SNES/MemoryManager.h"
#include "SNES/Console.h"
#include "SNES/Debugger/TraceLogger/Cx4TraceLogger.h"
#include "SNES/Coprocessors/CX4/Cx4.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "SNES/MemoryMappings.h"
#include "MemoryOperationType.h"

Cx4Debugger::Cx4Debugger(Debugger* debugger)
{
	Console* console = (Console*)debugger->GetConsole();

	_debugger = debugger;
	_codeDataLogger = debugger->GetCodeDataLogger(CpuType::Cpu).get();
	_disassembler = debugger->GetDisassembler().get();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter().get();
	_cx4 = console->GetCartridge()->GetCx4();
	_memoryManager = console->GetMemoryManager().get();
	_settings = debugger->GetEmulator()->GetSettings();
	
	_traceLogger.reset(new Cx4TraceLogger(debugger, console->GetPpu().get(), _memoryManager));

	_breakpointManager.reset(new BreakpointManager(debugger, CpuType::Cx4, debugger->GetEventManager(CpuType::Cpu).get()));
	_step.reset(new StepRequest());
}

void Cx4Debugger::Reset()
{
}

void Cx4Debugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	Cx4State& state = _cx4->GetState();
	addr = (state.Cache.Address[state.Cache.Page] + (state.PC * 2)) & 0xFFFFFF;

	AddressInfo addressInfo = _cx4->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, SnesMemoryType::Cx4Memory);

	if(type == MemoryOperationType::ExecOpCode) {
		AddressInfo opCodeHighAddr = _cx4->GetMemoryMappings()->GetAbsoluteAddress(addr + 1);
		if(addressInfo.Type == SnesMemoryType::PrgRom) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Code | CdlFlags::Cx4);
			_codeDataLogger->SetFlags(addressInfo.Address + 1, CdlFlags::Code | CdlFlags::Cx4);
		}

		if(_traceLogger->IsEnabled() || _settings->CheckDebuggerFlag(DebuggerFlags::Cx4DebuggerEnabled)) {
			_disassembler->BuildCache(addressInfo, 0, CpuType::Cx4);

			if(_traceLogger->IsEnabled()) {
				DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::Cx4);
				_traceLogger->Log(state, disInfo, operation);
			}
		}

		_prevProgramCounter = addr;

		_step->ProcessCpuExec();
		
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
		_memoryAccessCounter->ProcessMemoryExec(opCodeHighAddr, _memoryManager->GetMasterClock());
	} else {
		if(addressInfo.Type == SnesMemoryType::PrgRom) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Data | CdlFlags::Cx4);
		}
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}
		_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());
	}

	_debugger->ProcessBreakConditions(_step->StepCount == 0, GetBreakpointManager(), operation, addressInfo);
}

void Cx4Debugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _cx4->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, SnesMemoryType::Cx4Memory);
	_debugger->ProcessBreakConditions(_step->StepCount == 0, GetBreakpointManager(), operation, addressInfo);
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

BreakpointManager* Cx4Debugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

shared_ptr<CallstackManager> Cx4Debugger::GetCallstackManager()
{
	throw std::runtime_error("Call stack not supported for CX4");
}

shared_ptr<IAssembler> Cx4Debugger::GetAssembler()
{
	throw std::runtime_error("Assembler not supported for CX4");
}

shared_ptr<BaseEventManager> Cx4Debugger::GetEventManager()
{
	throw std::runtime_error("Event manager not supported for CX4");
}

shared_ptr<CodeDataLogger> Cx4Debugger::GetCodeDataLogger()
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
