#include "stdafx.h"
#include "SNES/BaseCartridge.h"
#include "SNES/MemoryManager.h"
#include "SNES/Console.h"
#include "SNES/Coprocessors/DSP/NecDsp.h"
#include "SNES/Debugger/NecDspDebugger.h"
#include "SNES/Debugger/TraceLogger/NecDspTraceLogger.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
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
	Console* console = (Console*)debugger->GetConsole();

	_debugger = debugger;
	_disassembler = debugger->GetDisassembler();
	_dsp = console->GetCartridge()->GetDsp();
	_settings = debugger->GetEmulator()->GetSettings();
	
	_traceLogger.reset(new NecDspTraceLogger(debugger, console->GetPpu(), console->GetMemoryManager()));

	_breakpointManager.reset(new BreakpointManager(debugger, CpuType::NecDsp, debugger->GetEventManager(CpuType::Cpu)));
	_step.reset(new StepRequest());
}

void NecDspDebugger::Reset()
{
}

void NecDspDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = { (int32_t)addr, type == MemoryOperationType::ExecOpCode ? SnesMemoryType::DspProgramRom : SnesMemoryType::DspDataRom };
	MemoryOperationInfo operation(addr, value, type, SnesMemoryType::NecDspMemory);

	if(type == MemoryOperationType::ExecOpCode) {
		if(_traceLogger->IsEnabled() || _settings->CheckDebuggerFlag(DebuggerFlags::NecDspDebuggerEnabled)) {
			_disassembler->BuildCache(addressInfo, 0, CpuType::NecDsp);

			if(_traceLogger->IsEnabled()) {
				DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::NecDsp);
				_traceLogger->Log(_dsp->GetState(), disInfo, operation);
			}
		}

		_prevProgramCounter = addr;
		_step->ProcessCpuExec();
	}

	_debugger->ProcessBreakConditions(_step->StepCount == 0, GetBreakpointManager(), operation, addressInfo);
}

void NecDspDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo { (int32_t)addr, SnesMemoryType::DspDataRam }; //Writes never affect the DSP ROM
	MemoryOperationInfo operation(addr, value, type, SnesMemoryType::NecDspMemory);
	_debugger->ProcessBreakConditions(false, GetBreakpointManager(), operation, addressInfo);

	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation);
	}
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

CallstackManager* NecDspDebugger::GetCallstackManager()
{
	throw std::runtime_error("Callstack not supported for NEC DSP");
}

BreakpointManager* NecDspDebugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

IAssembler* NecDspDebugger::GetAssembler()
{
	throw std::runtime_error("Assembler not supported for NEC DSP");
}

BaseEventManager* NecDspDebugger::GetEventManager()
{
	throw std::runtime_error("Event manager not supported for NEC DSP");
}

CodeDataLogger* NecDspDebugger::GetCodeDataLogger()
{
	return nullptr;
}

BaseState& NecDspDebugger::GetState()
{
	return _dsp->GetState();
}

ITraceLogger* NecDspDebugger::GetTraceLogger()
{
	return _traceLogger.get();
}
