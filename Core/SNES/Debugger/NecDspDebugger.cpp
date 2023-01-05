#include "pch.h"
#include "SNES/BaseCartridge.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/SnesConsole.h"
#include "SNES/Coprocessors/DSP/NecDsp.h"
#include "SNES/Debugger/NecDspDebugger.h"
#include "SNES/Debugger/NecDspDisUtils.h"
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
#include "Shared/MemoryOperationType.h"

NecDspDebugger::NecDspDebugger(Debugger* debugger) : IDebugger(debugger->GetEmulator())
{
	SnesConsole* console = (SnesConsole*)debugger->GetConsole();

	_debugger = debugger;
	_disassembler = debugger->GetDisassembler();
	_dsp = console->GetCartridge()->GetDsp();
	_settings = debugger->GetEmulator()->GetSettings();
	
	_traceLogger.reset(new NecDspTraceLogger(debugger, this, console->GetPpu(), console->GetMemoryManager()));
	_callstackManager.reset(new CallstackManager(debugger, console));

	_breakpointManager.reset(new BreakpointManager(debugger, this, CpuType::NecDsp, debugger->GetEventManager(CpuType::Snes)));
	_step.reset(new StepRequest());
}

void NecDspDebugger::Reset()
{
	_callstackManager->Clear();
	_prevOpCode = 0;
}

void NecDspDebugger::ProcessInstruction()
{
	uint32_t pc = _dsp->GetState().PC * 3;
	uint32_t opCode = _dsp->GetOpCode(_dsp->GetState().PC);
	AddressInfo addressInfo = { (int32_t)pc, MemoryType::DspProgramRom };
	MemoryOperationInfo operation(pc, opCode, MemoryOperationType::ExecOpCode, MemoryType::NecDspMemory);

	_disassembler->BuildCache(addressInfo, 0, CpuType::NecDsp);

	if(NecDspDisUtils::IsJumpToSub(_prevOpCode)) {
		//CALL and RST, and PC doesn't match the next instruction, so the call was (probably) done
		uint32_t returnPc = _prevProgramCounter + NecDspDisUtils::GetOpSize();
		AddressInfo src = { (int32_t)_prevProgramCounter, MemoryType::DspProgramRom };
		AddressInfo ret = { (int32_t)returnPc, MemoryType::DspProgramRom };
		_callstackManager->Push(src, _prevProgramCounter, addressInfo, pc, ret, returnPc, StackFrameFlags::None);
	} else if(NecDspDisUtils::IsReturnInstruction(_prevOpCode)) {
		_callstackManager->Pop(addressInfo, pc);

		if(_step->BreakAddress == (int32_t)pc) {
			//If we're on the expected return address, break immediately (for step over/step out)
			_step->Break(BreakSource::CpuStep);
		}
	}

	_prevProgramCounter = pc;
	_prevOpCode = opCode;

	_step->ProcessCpuExec();
	_debugger->ProcessBreakConditions(CpuType::NecDsp, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void NecDspDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = { (int32_t)addr, MemoryType::DspProgramRom };
	MemoryOperationInfo operation(addr, value, MemoryOperationType::ExecOpCode, MemoryType::NecDspMemory);

	if(_traceLogger->IsEnabled()) {
		DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::NecDsp);
		_traceLogger->Log(_dsp->GetState(), disInfo, operation, addressInfo);
	}
}

void NecDspDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	//TODOv2
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
		
		case StepType::StepOut: step.BreakAddress = _callstackManager->GetReturnAddress(); break;

		case StepType::StepOver:
			if(NecDspDisUtils::IsJumpToSub(_prevOpCode)) {
				step.BreakAddress = _prevProgramCounter + NecDspDisUtils::GetOpSize();
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

DebuggerFeatures NecDspDebugger::GetSupportedFeatures()
{
	DebuggerFeatures features = {};
	features.ChangeProgramCounter = AllowChangeProgramCounter;
	features.StepOut = true;
	features.StepOver = true;
	return features;
}

void NecDspDebugger::SetProgramCounter(uint32_t addr, bool updateDebuggerOnly)
{
	if(!updateDebuggerOnly) {
		_dsp->GetState().PC = addr / 3;
	}
	_prevProgramCounter = addr;
}

uint32_t NecDspDebugger::GetProgramCounter(bool getInstPc)
{
	return getInstPc ? _prevProgramCounter : (_dsp->GetState().PC * 3);
}

CallstackManager* NecDspDebugger::GetCallstackManager()
{
	return _callstackManager.get();
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

BaseState& NecDspDebugger::GetState()
{
	return _dsp->GetState();
}

ITraceLogger* NecDspDebugger::GetTraceLogger()
{
	return _traceLogger.get();
}
