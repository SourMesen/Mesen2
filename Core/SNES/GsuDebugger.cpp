#include "stdafx.h"
#include "GsuDebugger.h"
#include "DisassemblyInfo.h"
#include "Disassembler.h"
#include "BaseCartridge.h"
#include "Gsu.h"
#include "TraceLogger.h"
#include "CallstackManager.h"
#include "BreakpointManager.h"
#include "ExpressionEvaluator.h"
#include "MemoryManager.h"
#include "Debugger.h"
#include "Console.h"
#include "Emulator.h"
#include "EmuSettings.h"
#include "MemoryAccessCounter.h"
#include "CodeDataLogger.h"
#include "MemoryOperationType.h"

GsuDebugger::GsuDebugger(Debugger* debugger)
{
	_debugger = debugger;
	_codeDataLogger = debugger->GetCodeDataLogger(CpuType::Cpu).get();
	_traceLogger = debugger->GetTraceLogger().get();
	_disassembler = debugger->GetDisassembler().get();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter().get();
	_gsu = ((Console*)debugger->GetConsole())->GetCartridge()->GetGsu();
	_memoryManager = ((Console*)debugger->GetConsole())->GetMemoryManager().get();
	_settings = debugger->GetEmulator()->GetSettings().get();

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

		if(_traceLogger->IsCpuLogged(CpuType::Gsu) || _settings->CheckDebuggerFlag(DebuggerFlags::GsuDebuggerEnabled)) {
			GsuState gsuState = _gsu->GetState();
			_disassembler->BuildCache(addressInfo, gsuState.SFR.GetFlagsHigh() & 0x13, CpuType::Gsu);

			if(_traceLogger->IsCpuLogged(CpuType::Gsu)) {
				gsuState.R[15] = addr;

				DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::Gsu);
				_traceLogger->Log(CpuType::Gsu, gsuState, disInfo);
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
		_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());
	}

	_debugger->ProcessBreakConditions(_step->StepCount == 0, GetBreakpointManager(), operation, addressInfo);
}

void GsuDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _gsu->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation { addr, value, type };
	_debugger->ProcessBreakConditions(false, GetBreakpointManager(), operation, addressInfo);

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
	throw std::runtime_error("CDL not supported for GSU");
}

BaseState& GsuDebugger::GetState()
{
	return _gsu->GetState();
}
