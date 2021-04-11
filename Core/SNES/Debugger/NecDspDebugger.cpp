#include "stdafx.h"
#include "SNES/BaseCartridge.h"
#include "SNES/MemoryManager.h"
#include "SNES/Console.h"
#include "SNES/Coprocessors/DSP/NecDsp.h"
#include "SNES/Debugger/NecDspDebugger.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
#include "Debugger/TraceLogger.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/Debugger.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "MemoryOperationType.h"

NecDspDebugger::NecDspDebugger(Debugger* debugger)
{
	_debugger = debugger;
	_traceLogger = debugger->GetTraceLogger().get();
	_disassembler = debugger->GetDisassembler().get();
	_dsp = ((Console*)debugger->GetConsole())->GetCartridge()->GetDsp();
	_settings = debugger->GetEmulator()->GetSettings();

	_breakpointManager.reset(new BreakpointManager(debugger, CpuType::NecDsp));
	_step.reset(new StepRequest());
}

void NecDspDebugger::Reset()
{
}

void NecDspDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = { (int32_t)addr, type == MemoryOperationType::ExecOpCode ? SnesMemoryType::DspProgramRom : SnesMemoryType::DspDataRom };
	MemoryOperationInfo operation { (uint32_t)addr, value, type };

	if(type == MemoryOperationType::ExecOpCode) {
		if(_traceLogger->IsCpuLogged(CpuType::NecDsp) || _settings->CheckDebuggerFlag(DebuggerFlags::NecDspDebuggerEnabled)) {
			_disassembler->BuildCache(addressInfo, 0, CpuType::NecDsp);

			if(_traceLogger->IsCpuLogged(CpuType::NecDsp)) {
				DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::NecDsp);
				NecDspState state = _dsp->GetState();
				_traceLogger->Log(CpuType::NecDsp, state, disInfo);
			}
		}

		_prevProgramCounter = addr;

		if(_step->StepCount > 0) {
			_step->StepCount--;
		}
	}

	_debugger->ProcessBreakConditions(_step->StepCount == 0, GetBreakpointManager(), operation, addressInfo);
}

void NecDspDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo { (int32_t)addr, SnesMemoryType::DspDataRam }; //Writes never affect the DSP ROM
	MemoryOperationInfo operation { addr, value, type };
	_debugger->ProcessBreakConditions(false, GetBreakpointManager(), operation, addressInfo);
}

void NecDspDebugger::Run()
{
	_step.reset(new StepRequest());
}

void NecDspDebugger::Step(int32_t stepCount, StepType type)
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

shared_ptr<CallstackManager> NecDspDebugger::GetCallstackManager()
{
	throw std::runtime_error("Callstack not supported for NEC DSP");
}

BreakpointManager* NecDspDebugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

shared_ptr<IAssembler> NecDspDebugger::GetAssembler()
{
	throw std::runtime_error("Assembler not supported for NEC DSP");
}

shared_ptr<IEventManager> NecDspDebugger::GetEventManager()
{
	throw std::runtime_error("Event manager not supported for NEC DSP");
}

shared_ptr<CodeDataLogger> NecDspDebugger::GetCodeDataLogger()
{
	throw std::runtime_error("CDL not supported for NEC DSP");
}

BaseState& NecDspDebugger::GetState()
{
	return _dsp->GetState();
}
