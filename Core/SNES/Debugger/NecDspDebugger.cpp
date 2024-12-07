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
	_memoryAccessCounter = debugger->GetMemoryAccessCounter();
	_dsp = console->GetCartridge()->GetDsp();
	_settings = debugger->GetEmulator()->GetSettings();
	_memoryManager = console->GetMemoryManager();

	_traceLogger.reset(new NecDspTraceLogger(debugger, this, console->GetPpu(), console->GetMemoryManager()));
	_callstackManager.reset(new CallstackManager(debugger, this));

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
	NecDspState& state = _dsp->GetState();
	uint32_t pc = state.PC * 3;
	uint32_t opCode = _dsp->GetOpCode(state.PC);
	AddressInfo addressInfo = { (int32_t)pc, MemoryType::DspProgramRom };
	MemoryOperationInfo operation(pc, opCode, MemoryOperationType::ExecOpCode, MemoryType::NecDspMemory);
	InstructionProgress.LastMemOperation = operation;
	InstructionProgress.StartCycle = state.CycleCount;

	_disassembler->BuildCache(addressInfo, 0, CpuType::NecDsp);

	if(NecDspDisUtils::IsJumpToSub(_prevOpCode)) {
		//CALL and RST, and PC doesn't match the next instruction, so the call was (probably) done
		uint32_t returnPc = _prevProgramCounter + NecDspDisUtils::GetOpSize();
		AddressInfo src = { (int32_t)_prevProgramCounter, MemoryType::DspProgramRom };
		AddressInfo ret = { (int32_t)returnPc, MemoryType::DspProgramRom };
		_callstackManager->Push(src, _prevProgramCounter, addressInfo, pc, ret, returnPc, _prevStackPointer, StackFrameFlags::None);
	} else if(NecDspDisUtils::IsReturnInstruction(_prevOpCode)) {
		_callstackManager->Pop(addressInfo, pc, state.SP);

		if(_step->BreakAddress == (int32_t)pc && _step->BreakStackPointer == state.SP) {
			//If we're on the expected return address, break immediately (for step over/step out)
			_step->Break(BreakSource::CpuStep);
		}
	}

	_prevProgramCounter = pc;
	_prevOpCode = opCode;
	_prevStackPointer = state.SP;

	_step->ProcessCpuExec();
	_debugger->ProcessBreakConditions(CpuType::NecDsp, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void NecDspDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	if(type == MemoryOperationType::ExecOpCode) {
		AddressInfo addressInfo = { (int32_t)addr, MemoryType::DspProgramRom };
		MemoryOperationInfo operation(addr, value, MemoryOperationType::ExecOpCode, MemoryType::NecDspMemory);
		InstructionProgress.LastMemOperation = operation;

		if(_traceLogger->IsEnabled()) {
			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::NecDsp);
			_traceLogger->Log(_dsp->GetState(), disInfo, operation, addressInfo);
		}
	} else {
		MemoryType memType = (addr & NecDsp::DataRomReadFlag) ? MemoryType::DspDataRom : MemoryType::DspDataRam;
		addr &= ~NecDsp::DataRomReadFlag;

		AddressInfo addressInfo = { (int32_t)addr, memType };
		MemoryOperationInfo operation(addr, value, type, memType);
		InstructionProgress.LastMemOperation = operation;

		_debugger->ProcessBreakConditions(CpuType::NecDsp, *_step.get(), _breakpointManager.get(), operation, addressInfo);
		_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation, addressInfo);
		}
	}
}

void NecDspDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = { (int32_t)addr, MemoryType::DspDataRam };
	MemoryOperationInfo operation(addr, value, type, MemoryType::DspDataRam);
	InstructionProgress.LastMemOperation = operation;
	_debugger->ProcessBreakConditions(CpuType::NecDsp, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetMasterClock());
	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation, addressInfo);
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
			step.BreakAddress = _callstackManager->GetReturnAddress();
			step.BreakStackPointer = _callstackManager->GetReturnStackPointer();
			break;

		case StepType::StepOver:
			if(NecDspDisUtils::IsJumpToSub(_prevOpCode)) {
				step.BreakAddress = _prevProgramCounter + NecDspDisUtils::GetOpSize();
				step.BreakStackPointer = _prevStackPointer;
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
	_prevStackPointer = _dsp->GetState().SP;
}

uint32_t NecDspDebugger::GetProgramCounter(bool getInstPc)
{
	return getInstPc ? _prevProgramCounter : (_dsp->GetState().PC * 3);
}

uint64_t NecDspDebugger::GetCpuCycleCount(bool forProfiler)
{
	return _dsp->GetState().CycleCount;
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
