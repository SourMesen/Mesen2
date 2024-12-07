#include "pch.h"
#include "SNES/Spc.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/SnesConsole.h"
#include "SNES/Debugger/SpcDebugger.h"
#include "SNES/Debugger/SpcDisUtils.h"
#include "SNES/Debugger/DummySpc.h"
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
#include "Shared/MemoryOperationType.h"

SpcDebugger::SpcDebugger(Debugger* debugger) : IDebugger(debugger->GetEmulator())
{
	_debugger = debugger;
	_disassembler = debugger->GetDisassembler();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter();

	SnesConsole* console = (SnesConsole*)debugger->GetConsole();
	_spc = console->GetSpc();
	_memoryManager = console->GetMemoryManager();
	_settings = debugger->GetEmulator()->GetSettings();

	_dummyCpu.reset(new DummySpc(_spc->GetSpcRam()));

	_traceLogger.reset(new SpcTraceLogger(debugger, this, console->GetPpu(), console->GetMemoryManager()));

	_callstackManager.reset(new CallstackManager(debugger, this));
	_breakpointManager.reset(new BreakpointManager(debugger, this, CpuType::Spc, debugger->GetEventManager(CpuType::Snes)));
	_step.reset(new StepRequest());
}

void SpcDebugger::Reset()
{
	_callstackManager->Clear();
	_prevOpCode = 0xFF;
}

void SpcDebugger::ProcessConfigChange()
{
	_debuggerEnabled = _settings->CheckDebuggerFlag(DebuggerFlags::SpcDebuggerEnabled);
	_predictiveBreakpoints = _settings->GetDebugConfig().UsePredictiveBreakpoints;
	_ignoreDspReadWrites = _settings->GetDebugConfig().SnesIgnoreDspReadWrites;
}

void SpcDebugger::ProcessInstruction()
{
	SpcState& state = _spc->GetState();
	uint16_t addr = state.PC;
	uint8_t value = _spc->DebugRead(addr);
	AddressInfo addressInfo = _spc->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, MemoryOperationType::ExecOpCode, MemoryType::SpcMemory);
	InstructionProgress.LastMemOperation = operation;
	InstructionProgress.StartCycle = state.Cycle;

	_disassembler->BuildCache(addressInfo, 0, CpuType::Spc);

	if(SpcDisUtils::IsJumpToSub(_prevOpCode)) {
		//JSR, BRK, PCALL, TCALL
		uint8_t opSize = SpcDisUtils::GetOpSize(_prevOpCode);
		uint16_t returnPc = _prevProgramCounter + opSize;
		AddressInfo src = _spc->GetAbsoluteAddress(_prevProgramCounter);
		AddressInfo ret = _spc->GetAbsoluteAddress(returnPc);
		_callstackManager->Push(src, _prevProgramCounter, addressInfo, addr, ret, returnPc, _prevStackPointer, StackFrameFlags::None);
	} else if(SpcDisUtils::IsReturnInstruction(_prevOpCode)) {
		//RTS, RTI
		_callstackManager->Pop(addressInfo, addr, state.SP);

		if(_step->BreakAddress == (int32_t)addr && _step->BreakStackPointer == state.SP) {
			//RTS/RTI - if we're on the expected return address, break immediately (for step over/step out)
			_step->Break(BreakSource::CpuStep);
		}
	}

	_prevOpCode = value;
	_prevProgramCounter = addr;
	_prevStackPointer = state.SP;

	_step->ProcessCpuExec();

	if(_debuggerEnabled) {
		//Break on BRK/STP
		if(value == 0x0F && _settings->GetDebugConfig().SnesBreakOnBrk) {
			_step->Break(BreakSource::BreakOnBrk);
		} else if(value == 0xFF && _settings->GetDebugConfig().SnesBreakOnStp) {
			_step->Break(BreakSource::BreakOnStp);
		}
	}

	if(_step->StepCount != 0 && _breakpointManager->HasBreakpoints() && _predictiveBreakpoints) {
		_dummyCpu->SetDummyState(state);
		_dummyCpu->Step();
		for(uint32_t i = 1; i < _dummyCpu->GetOperationCount(); i++) {
			MemoryOperationInfo memOp = _dummyCpu->GetOperationInfo(i);
			if(_breakpointManager->HasBreakpointForType(memOp.Type)) {
				AddressInfo absAddr = _spc->GetAbsoluteAddress(memOp.Address);
				_debugger->ProcessPredictiveBreakpoint(CpuType::Spc, _breakpointManager.get(), memOp, absAddr);
			}
		}
	}

	_debugger->ProcessBreakConditions(CpuType::Spc, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

template<MemoryAccessFlags flags>
void SpcDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	MemoryOperationInfo operation(addr, value, type, MemoryType::SpcMemory);
	InstructionProgress.LastMemOperation = operation;

	if constexpr(flags == MemoryAccessFlags::None) {
		//SPC read
		AddressInfo addressInfo = _spc->GetAbsoluteAddress(addr);

		if(type == MemoryOperationType::ExecOpCode) {
			if(_traceLogger->IsEnabled()) {
				SpcState& state = _spc->GetState();
				DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::Spc);
				_traceLogger->Log(state, disInfo, operation, addressInfo);
			}
			_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
		} else if(type == MemoryOperationType::ExecOperand) {
			_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
			if(_traceLogger->IsEnabled()) {
				_traceLogger->LogNonExec(operation, addressInfo);
			}
			_debugger->ProcessBreakConditions(CpuType::Spc, *_step.get(), _breakpointManager.get(), operation, addressInfo);
		} else {
			_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());
			if(_traceLogger->IsEnabled()) {
				_traceLogger->LogNonExec(operation, addressInfo);
			}
			_debugger->ProcessBreakConditions(CpuType::Spc, *_step.get(), _breakpointManager.get(), operation, addressInfo);
		}
	} else {
		//DSP read
		if(!_ignoreDspReadWrites) {
			AddressInfo addressInfo { (int32_t)addr, MemoryType::SpcRam }; //DSP reads never read from the IPL ROM

			_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());
			_debugger->ProcessBreakConditions(CpuType::Spc, *_step.get(), _breakpointManager.get(), operation, addressInfo);
		}
	}
}

template<MemoryAccessFlags flags>
void SpcDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo { (int32_t)addr, MemoryType::SpcRam }; //Writes never affect the IPL ROM
	MemoryOperationInfo operation(addr, value, type, MemoryType::SpcMemory);
	InstructionProgress.LastMemOperation = operation;

	//Always invalidate cache, even if DSP writes are ignored
	_disassembler->InvalidateCache(addressInfo, CpuType::Spc);
	
	if constexpr(flags == MemoryAccessFlags::None) {
		//SPC write
		_debugger->ProcessBreakConditions(CpuType::Spc, *_step.get(), _breakpointManager.get(), operation, addressInfo);
		_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetMasterClock());

		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation, addressInfo);
		}
	} else {
		//DSP write
		if(!_ignoreDspReadWrites) {
			_debugger->ProcessBreakConditions(CpuType::Spc, *_step.get(), _breakpointManager.get(), operation, addressInfo);
			_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetMasterClock());
		}
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
		case StepType::StepOut:
			step.BreakAddress = _callstackManager->GetReturnAddress();
			step.BreakStackPointer = _callstackManager->GetReturnStackPointer();
			break;

		case StepType::StepOver:
			if(_prevOpCode == 0x3F || _prevOpCode == 0x0F || _prevOpCode == 0x4F || (_prevOpCode&0x0F) == 0x01) {
				//JSR, BRK, PCALL, TCALL
				step.BreakAddress = _prevProgramCounter + SpcDisUtils::GetOpSize(_prevOpCode);
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

DebuggerFeatures SpcDebugger::GetSupportedFeatures()
{
	DebuggerFeatures features = {};
	features.RunToIrq = false;
	features.RunToNmi = false;
	features.StepOver = true;
	features.StepOut = true;
	features.CallStack = true;
	features.ChangeProgramCounter = AllowChangeProgramCounter;
	return features;
}

void SpcDebugger::SetProgramCounter(uint32_t addr, bool updateDebuggerOnly)
{
	if(!updateDebuggerOnly) {
		_spc->GetState().PC = (uint16_t)addr;
	}
	_prevOpCode = _spc->DebugRead(addr);
	_prevProgramCounter = (uint16_t)addr;
	_prevStackPointer = _spc->GetState().SP;
}

uint32_t SpcDebugger::GetProgramCounter(bool getInstPc)
{
	return getInstPc ? _prevProgramCounter : _spc->GetState().PC;
}

uint64_t SpcDebugger::GetCpuCycleCount(bool forProfiler)
{
	return _spc->GetState().Cycle;
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

template void SpcDebugger::ProcessRead<MemoryAccessFlags::None>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void SpcDebugger::ProcessRead<MemoryAccessFlags::DspAccess>(uint32_t addr, uint8_t value, MemoryOperationType opType);

template void SpcDebugger::ProcessWrite<MemoryAccessFlags::None>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void SpcDebugger::ProcessWrite<MemoryAccessFlags::DspAccess>(uint32_t addr, uint8_t value, MemoryOperationType opType);