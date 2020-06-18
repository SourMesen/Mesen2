#include "stdafx.h"
#include "NecDsp.h"
#include "Cx4Debugger.h"
#include "DisassemblyInfo.h"
#include "Disassembler.h"
#include "TraceLogger.h"
#include "CallstackManager.h"
#include "BreakpointManager.h"
#include "BaseCartridge.h"
#include "MemoryManager.h"
#include "Debugger.h"
#include "Console.h"
#include "MemoryAccessCounter.h"
#include "ExpressionEvaluator.h"
#include "CodeDataLogger.h"
#include "EmuSettings.h"
#include "Cx4.h"
#include "MemoryMappings.h"

Cx4Debugger::Cx4Debugger(Debugger* debugger)
{
	_debugger = debugger;
	_codeDataLogger = debugger->GetCodeDataLogger(CpuType::Cpu).get();
	_traceLogger = debugger->GetTraceLogger().get();
	_disassembler = debugger->GetDisassembler().get();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter().get();
	_cx4 = debugger->GetConsole()->GetCartridge()->GetCx4();
	_memoryManager = debugger->GetConsole()->GetMemoryManager().get();
	_settings = debugger->GetConsole()->GetSettings().get();

	_breakpointManager.reset(new BreakpointManager(debugger, CpuType::Cx4));
	_step.reset(new StepRequest());
}

void Cx4Debugger::Reset()
{
}

void Cx4Debugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	Cx4State state = _cx4->GetState();
	addr = (state.Cache.Address[state.Cache.Page] + (state.PC * 2)) & 0xFFFFFF;

	AddressInfo addressInfo = _cx4->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation { (uint32_t)addr, value, type };

	if(type == MemoryOperationType::ExecOpCode) {
		AddressInfo opCodeHighAddr = _cx4->GetMemoryMappings()->GetAbsoluteAddress(addr + 1);
		if(addressInfo.Type == SnesMemoryType::PrgRom) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Code | CdlFlags::Cx4);
			_codeDataLogger->SetFlags(addressInfo.Address + 1, CdlFlags::Code | CdlFlags::Cx4);
		}

		if(_traceLogger->IsCpuLogged(CpuType::Cx4) || _settings->CheckDebuggerFlag(DebuggerFlags::Cx4DebuggerEnabled)) {
			_disassembler->BuildCache(addressInfo, 0, CpuType::Cx4);

			if(_traceLogger->IsCpuLogged(CpuType::Cx4)) {
				DebugState debugState;
				_debugger->GetState(debugState, true);

				DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::Cx4);
				_traceLogger->Log(CpuType::Cx4, debugState, disInfo);
			}
		}

		_prevProgramCounter = addr;

		if(_step->StepCount > 0) {
			_step->StepCount--;
		}

		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
		_memoryAccessCounter->ProcessMemoryExec(opCodeHighAddr, _memoryManager->GetMasterClock());
	} else {
		if(addressInfo.Type == SnesMemoryType::PrgRom) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Data | CdlFlags::Cx4);
		}
		_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());
	}

	_debugger->ProcessBreakConditions(_step->StepCount == 0, GetBreakpointManager(), operation, addressInfo);
}

void Cx4Debugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _cx4->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation { (uint32_t)addr, value, type };
	_debugger->ProcessBreakConditions(_step->StepCount == 0, GetBreakpointManager(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetMasterClock());
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