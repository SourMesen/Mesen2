#include "stdafx.h"
#include "SNES/Debugger/GsuDebugger.h"
#include "SNES/MemoryManager.h"
#include "SNES/Console.h"
#include "SNES/BaseCartridge.h"
#include "SNES/Coprocessors/GSU/Gsu.h"
#include "SNES/Debugger/TraceLogger/GsuTraceLogger.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Debugger/Debugger.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/CodeDataLogger.h"
#include "MemoryOperationType.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"

GsuDebugger::GsuDebugger(Debugger* debugger)
{
	Console* console = (Console*)debugger->GetConsole();

	_debugger = debugger;
	_codeDataLogger = debugger->GetCodeDataLogger(CpuType::Cpu).get();
	_disassembler = debugger->GetDisassembler().get();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter().get();
	_gsu = console->GetCartridge()->GetGsu();
	_memoryManager = console->GetMemoryManager().get();
	_settings = debugger->GetEmulator()->GetSettings();
	
	_traceLogger.reset(new GsuTraceLogger(debugger, console->GetPpu().get(), _memoryManager));

	_breakpointManager.reset(new BreakpointManager(debugger, CpuType::Gsu));
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
		if(addressInfo.Type == SnesMemoryType::PrgRom) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Code | CdlFlags::Gsu);
		}

		if(_traceLogger->IsEnabled() || _settings->CheckDebuggerFlag(DebuggerFlags::GsuDebuggerEnabled)) {
			GsuState gsuState = _gsu->GetState();
			_disassembler->BuildCache(addressInfo, gsuState.SFR.GetFlagsHigh() & 0x13, CpuType::Gsu);

			if(_traceLogger->IsEnabled()) {
				gsuState.R[15] = addr;

				DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::Gsu);
				_traceLogger->Log(gsuState, disInfo, operation);
			}
		}

		_prevOpCode = value;
		_prevProgramCounter = addr;

		if(_step->StepCount > 0) {
			_step->StepCount--;
		}
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
	} else {
		if(addressInfo.Type == SnesMemoryType::PrgRom) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Data | CdlFlags::Gsu);
		}
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}
		_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());
	}

	_debugger->ProcessBreakConditions(_step->StepCount == 0, GetBreakpointManager(), operation, addressInfo);
}

void GsuDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _gsu->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation { addr, value, type };
	_debugger->ProcessBreakConditions(false, GetBreakpointManager(), operation, addressInfo);

	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation);
	}

	_disassembler->InvalidateCache(addressInfo, CpuType::Gsu);
	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetMasterClock());
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

BreakpointManager* GsuDebugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

shared_ptr<CallstackManager> GsuDebugger::GetCallstackManager()
{
	throw std::runtime_error("Call stack not supported for GSU");
}

shared_ptr<IAssembler> GsuDebugger::GetAssembler()
{
	throw std::runtime_error("Assembler not supported for GSU");
}

shared_ptr<IEventManager> GsuDebugger::GetEventManager()
{
	throw std::runtime_error("Event manager not supported for GSU");
}

shared_ptr<CodeDataLogger> GsuDebugger::GetCodeDataLogger()
{
	return nullptr;
}

BaseState& GsuDebugger::GetState()
{
	return _gsu->GetState();
}

ITraceLogger* GsuDebugger::GetTraceLogger()
{
	return _traceLogger.get();
}
