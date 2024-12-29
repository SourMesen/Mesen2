#include "pch.h"
#include "SNES/Debugger/St018Debugger.h"
#include "SNES/Debugger/DummyArmV3Cpu.h"
#include "SNES/Debugger/TraceLogger/St018TraceLogger.h"
#include "SNES/Coprocessors/ST018/ArmV3Cpu.h"
#include "SNES/Coprocessors/ST018/ArmV3Types.h"
#include "SNES/Coprocessors/ST018/St018.h"
#include "SNES/BaseCartridge.h"
#include "SNES/SnesConsole.h"
#include "GBA/Debugger/GbaDisUtils.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/Debugger.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Debugger/MemoryDumper.h"
#include "Debugger/CodeDataLogger.h"
#include "Debugger/BaseEventManager.h"
#include "Utilities/HexUtilities.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"
#include "Shared/BaseControlManager.h"
#include "Shared/MemoryOperationType.h"

St018Debugger::St018Debugger(Debugger* debugger) : IDebugger(debugger->GetEmulator())
{
	SnesConsole* console = (SnesConsole*)debugger->GetConsole();

	_debugger = debugger;
	_emu = debugger->GetEmulator();

	_disassembler = debugger->GetDisassembler();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter();

	_console = console;
	
	_st018 = _console->GetCartridge()->GetSt018();
	_cpu = _st018->GetCpu();

	_settings = debugger->GetEmulator()->GetSettings();
	
	_traceLogger.reset(new St018TraceLogger(debugger, this, console->GetPpu()));
	
	_callstackManager.reset(new CallstackManager(debugger, this));
	_breakpointManager.reset(new BreakpointManager(debugger, this, CpuType::St018, debugger->GetEventManager(CpuType::St018)));
	_step.reset(new StepRequest());
	
	_dummyCpu.reset(new DummyArmV3Cpu());
	_dummyCpu->Init(_emu, _st018);
}

St018Debugger::~St018Debugger()
{
}

void St018Debugger::Reset()
{
	_callstackManager->Clear();
	ResetPrevOpCode();
}

void St018Debugger::ProcessInstruction()
{
	ArmV3CpuState& state = _cpu->GetState();
	uint32_t pc = state.Pipeline.Execute.Address;
	uint32_t opCode = state.Pipeline.Execute.OpCode;
	
	uint8_t flags = 0;

	AddressInfo addressInfo = _st018->GetArmAbsoluteAddress(pc);
	MemoryOperationInfo operation(pc, opCode, MemoryOperationType::ExecOpCode, MemoryType::St018Memory);
	InstructionProgress.LastMemOperation = operation;
	InstructionProgress.StartCycle = _cpu->GetState().CycleCount;

	if(addressInfo.Type != MemoryType::None) {
		_disassembler->BuildCache(addressInfo, flags, CpuType::St018);
	}

	ProcessCallStackUpdates(addressInfo, pc);

	_prevFlags = flags;
	_prevOpCode = opCode;
	_prevProgramCounter = pc;

	_step->ProcessCpuExec();

	if(_step->StepCount != 0 && _breakpointManager->HasBreakpoints() && _settings->GetDebugConfig().UsePredictiveBreakpoints) {
		_dummyCpu->SetDummyState(state);
		_dummyCpu->Exec();
		for(uint32_t i = 1; i < _dummyCpu->GetOperationCount(); i++) {
			MemoryOperationInfo memOp = _dummyCpu->GetOperationInfo(i);
			if(_breakpointManager->HasBreakpointForType(memOp.Type)) {
				AddressInfo absAddr = _st018->GetArmAbsoluteAddress(memOp.Address);
				switch(_dummyCpu->GetOperationMode(i) & (ArmV3AccessMode::Byte | ArmV3AccessMode::Word)) {
					case ArmV3AccessMode::Byte: _debugger->ProcessPredictiveBreakpoint<1>(CpuType::St018, _breakpointManager.get(), memOp, absAddr); break;
					case ArmV3AccessMode::Word: _debugger->ProcessPredictiveBreakpoint<4>(CpuType::St018, _breakpointManager.get(), memOp, absAddr); break;
				}
			}
		}
	}

	_debugger->ProcessBreakConditions<4>(CpuType::St018, *_step.get(), _breakpointManager.get(), operation, addressInfo);

	if(_traceLogger->IsEnabled()) {
		DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, pc, _prevFlags, CpuType::St018);
		_traceLogger->Log(state, disInfo, operation, addressInfo);
	}
}

template<uint8_t accessWidth>
void St018Debugger::ProcessRead(uint32_t addr, uint32_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _st018->GetArmAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::St018Memory);
	InstructionProgress.LastMemOperation = operation;

	if(type == MemoryOperationType::ExecOpCode) {
		_memoryAccessCounter->ProcessMemoryExec<accessWidth>(addressInfo, _console->GetMasterClock());
	} else {
		if(addressInfo.Address >= 0) {
			ReadResult result = _memoryAccessCounter->ProcessMemoryRead<accessWidth>(addressInfo, _console->GetMasterClock());
			if(result != ReadResult::Normal) {
				//Memory access was a read on an uninitialized memory address
				if((int)result & (int)ReadResult::FirstUninitRead) {
					//Only warn the first time
					_debugger->Log("[ST018] Uninitialized memory read: $" + HexUtilities::ToHex(addr));
				}
				if(_settings->CheckDebuggerFlag(DebuggerFlags::St018DebuggerEnabled) && _settings->GetDebugConfig().BreakOnUninitRead) {
					_step->Break(BreakSource::BreakOnUninitMemoryRead);
				}
			}
		}

		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation, addressInfo);
		}

		_debugger->ProcessBreakConditions<accessWidth>(CpuType::St018, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	}
}

template<uint8_t accessWidth>
void St018Debugger::ProcessWrite(uint32_t addr, uint32_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _st018->GetArmAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::St018Memory);
	InstructionProgress.LastMemOperation = operation;
	_debugger->ProcessBreakConditions<accessWidth>(CpuType::St018, *_step.get(), _breakpointManager.get(), operation, addressInfo);

	if(addressInfo.Type == MemoryType::St018WorkRam) {
		_disassembler->InvalidateCache(addressInfo, CpuType::St018);
	}

	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation, addressInfo);
	}

	_memoryAccessCounter->ProcessMemoryWrite<accessWidth>(addressInfo, _console->GetMasterClock());
}

void St018Debugger::Run()
{
	_step.reset(new StepRequest());
}

void St018Debugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step(type);

	switch(type) {
		case StepType::Step: step.StepCount = stepCount; break;
		case StepType::StepOut: step.BreakAddress = _callstackManager->GetReturnAddress(); break;
		case StepType::StepOver:
			if(GbaDisUtils::IsJumpToSub(_prevOpCode, _prevFlags)) {
				step.BreakAddress = _prevProgramCounter + GbaDisUtils::GetOpSize(_prevOpCode, _prevFlags);
			} else {
				//For any other instruction, step over is the same as step into
				step.StepCount = 1;
			}
			break;
	}

	_step.reset(new StepRequest(step));
}

void St018Debugger::ProcessCallStackUpdates(AddressInfo& destAddr, uint32_t destPc)
{
	if(GbaDisUtils::IsJumpToSub(_prevOpCode, _prevFlags) && destPc != _prevProgramCounter + GbaDisUtils::GetOpSize(_prevOpCode, _prevFlags)) {
		//New PC doesn't match the next instruction, so the call was (probably) done
		uint32_t returnPc = _prevProgramCounter + GbaDisUtils::GetOpSize(_prevOpCode, _prevFlags);
		AddressInfo src = _st018->GetArmAbsoluteAddress(_prevProgramCounter);
		AddressInfo ret = _st018->GetArmAbsoluteAddress(returnPc);
		_callstackManager->Push(src, _prevProgramCounter, destAddr, destPc, ret, returnPc, 0, StackFrameFlags::None);
	} else if(GbaDisUtils::IsUnconditionalJump(_prevOpCode, _prevFlags) || GbaDisUtils::IsConditionalJump(_prevOpCode, _prevFlags)) {
		if(destPc != _prevProgramCounter + GbaDisUtils::GetOpSize(_prevOpCode, _prevFlags)) {
			//Return instruction used, and PC doesn't match the next instruction, so the ret was (probably) taken (can be conditional)
			if(_callstackManager->IsReturnAddrMatch(destPc)) {
				//Only pop top of callstack if the address matches the expected address
				_callstackManager->Pop(destAddr, destPc, 0);
			}
		}

		if(_step->BreakAddress == destPc) {
			//If we're on the expected return address, break immediately (for step over/step out)
			_step->Break(BreakSource::CpuStep);
		}
	}
}

void St018Debugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	AddressInfo ret = _st018->GetArmAbsoluteAddress(originalPc);
	AddressInfo dest = _st018->GetArmAbsoluteAddress(currentPc);

	//If a call/return occurred just before IRQ, it needs to be processed now
	ProcessCallStackUpdates(ret, originalPc);
	ResetPrevOpCode();

	_debugger->InternalProcessInterrupt(
		CpuType::St018, *this, *_step.get(), 
		ret, originalPc, dest, currentPc, ret, originalPc, 0, forNmi
	);
}

DebuggerFeatures St018Debugger::GetSupportedFeatures()
{
	DebuggerFeatures features = {};
	features.ChangeProgramCounter = AllowChangeProgramCounter;
	features.StepOver = true;
	features.StepOut = true;
	features.CallStack = true;

	features.CpuVectorCount = 3;
	features.CpuVectors[0] = { "IRQ", (int)ArmV3CpuVector::Irq, VectorType::Direct };
	features.CpuVectors[1] = { "SWI", (int)ArmV3CpuVector::SoftwareIrq, VectorType::Direct };
	features.CpuVectors[2] = { "Undefined", (int)ArmV3CpuVector::Undefined, VectorType::Direct };

	return features;
}

void St018Debugger::SetProgramCounter(uint32_t addr, bool updateDebuggerOnly)
{
	if(!updateDebuggerOnly) {
		_cpu->SetProgramCounter(addr);
	}
	_prevOpCode = _st018->DebugCpuRead(ArmV3AccessMode::Word, addr);
	_prevProgramCounter = addr;
}

uint32_t St018Debugger::GetProgramCounter(bool getInstPc)
{
	return getInstPc ? _prevProgramCounter : _cpu->GetState().R[15];
}

uint64_t St018Debugger::GetCpuCycleCount(bool forProfiler)
{
	return _cpu->GetState().CycleCount;
}

void St018Debugger::ResetPrevOpCode()
{
	_prevOpCode = 0;
}

CallstackManager* St018Debugger::GetCallstackManager()
{
	return _callstackManager.get();
}

BreakpointManager* St018Debugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

BaseState& St018Debugger::GetState()
{
	return _cpu->GetState();
}

ITraceLogger* St018Debugger::GetTraceLogger()
{
	return _traceLogger.get();
}

IAssembler* St018Debugger::GetAssembler()
{
	throw std::runtime_error("Assembler not supported for ST018");
}

BaseEventManager* St018Debugger::GetEventManager()
{
	return nullptr;
}

template void St018Debugger::ProcessRead<1>(uint32_t addr, uint32_t value, MemoryOperationType type);
template void St018Debugger::ProcessRead<2>(uint32_t addr, uint32_t value, MemoryOperationType type);
template void St018Debugger::ProcessRead<4>(uint32_t addr, uint32_t value, MemoryOperationType type);

template void St018Debugger::ProcessWrite<1>(uint32_t addr, uint32_t value, MemoryOperationType type);
template void St018Debugger::ProcessWrite<2>(uint32_t addr, uint32_t value, MemoryOperationType type);
template void St018Debugger::ProcessWrite<4>(uint32_t addr, uint32_t value, MemoryOperationType type);
