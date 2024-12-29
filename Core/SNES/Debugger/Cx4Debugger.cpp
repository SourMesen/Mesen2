#include "pch.h"
#include "SNES/Debugger/Cx4Debugger.h"
#include "Debugger/CdlManager.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/Debugger.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/ExpressionEvaluator.h"
#include "SNES/BaseCartridge.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/SnesConsole.h"
#include "SNES/Debugger/SnesCodeDataLogger.h"
#include "SNES/Debugger/Cx4DisUtils.h"
#include "SNES/Debugger/TraceLogger/Cx4TraceLogger.h"
#include "SNES/Coprocessors/CX4/Cx4.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "SNES/MemoryMappings.h"
#include "Shared/MemoryOperationType.h"

Cx4Debugger::Cx4Debugger(Debugger* debugger) : IDebugger(debugger->GetEmulator())
{
	SnesConsole* console = (SnesConsole*)debugger->GetConsole();

	_debugger = debugger;
	_codeDataLogger = (SnesCodeDataLogger*)debugger->GetCdlManager()->GetCodeDataLogger(MemoryType::SnesPrgRom);
	_disassembler = debugger->GetDisassembler();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter();
	_cx4 = console->GetCartridge()->GetCx4();
	_memoryManager = console->GetMemoryManager();
	_settings = debugger->GetEmulator()->GetSettings();
	
	_traceLogger.reset(new Cx4TraceLogger(debugger, this, console->GetPpu(), _memoryManager));

	_breakpointManager.reset(new BreakpointManager(debugger, this, CpuType::Cx4, debugger->GetEventManager(CpuType::Snes)));
	_callstackManager.reset(new CallstackManager(debugger, this));
	_step.reset(new StepRequest());
}

void Cx4Debugger::Reset()
{
	_callstackManager->Clear();
	_prevOpCode = 0;
}

void Cx4Debugger::ProcessInstruction()
{
	Cx4State& state = _cx4->GetState();
	uint32_t pc = (state.Cache.Address[state.Cache.Page] + (state.PC * 2)) & 0xFFFFFF;
	MemoryMappings* mappings = _cx4->GetMemoryMappings();
	uint16_t opCode = mappings->PeekWord(pc);
	AddressInfo addressInfo = mappings->GetAbsoluteAddress(pc);
	MemoryOperationInfo operation(pc, opCode, MemoryOperationType::ExecOpCode, MemoryType::Cx4Memory);
	InstructionProgress.LastMemOperation = operation;
	InstructionProgress.StartCycle = state.CycleCount;

	if(addressInfo.Type == MemoryType::SnesPrgRom) {
		_codeDataLogger->SetCode<SnesCdlFlags::Cx4>(addressInfo.Address);
		_codeDataLogger->SetCode<SnesCdlFlags::Cx4>(addressInfo.Address + 1);
	}

	if(Cx4DisUtils::IsJumpToSub(_prevOpCode) && pc != _prevProgramCounter + Cx4DisUtils::GetOpSize()) {
		uint32_t returnPc = (_prevProgramCounter + Cx4DisUtils::GetOpSize()) & 0xFFFFFF;
		AddressInfo srcAddress = mappings->GetAbsoluteAddress(_prevProgramCounter);
		AddressInfo retAddress = mappings->GetAbsoluteAddress(returnPc);
		_callstackManager->Push(srcAddress, _prevProgramCounter, addressInfo, pc, retAddress, returnPc, _prevStackPointer, StackFrameFlags::None);
	} else if(Cx4DisUtils::IsReturnInstruction(_prevOpCode)) {
		_callstackManager->Pop(addressInfo, pc, state.SP);
		if(_step->BreakAddress == (int32_t)pc && _step->BreakStackPointer == state.SP) {
			//RTS - if we're on the expected return address, break immediately (for step over/step out)
			_step->Break(BreakSource::CpuStep);
		}
	}

	if(_settings->CheckDebuggerFlag(DebuggerFlags::Cx4DebuggerEnabled)) {
		_disassembler->BuildCache(addressInfo, 0, CpuType::Cx4);
	}

	_prevProgramCounter = pc;
	_prevStackPointer = state.SP;
	_prevOpCode = (opCode >> 8) & 0xFC;

	if(_prevOpCode == 0xFC) {
		//STOP instruction
		_callstackManager->Clear();
	}

	_step->ProcessCpuExec();
	_debugger->ProcessBreakConditions(CpuType::Cx4, *_step.get(), _breakpointManager.get(), operation, addressInfo);

	if(_traceLogger->IsEnabled()) {
		DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, pc, 0, CpuType::Cx4);
		_traceLogger->Log(state, disInfo, operation, addressInfo);
	}

	AddressInfo opCodeHighAddr = _cx4->GetMemoryMappings()->GetAbsoluteAddress(pc + 1);
	_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
	_memoryAccessCounter->ProcessMemoryExec(opCodeHighAddr, _memoryManager->GetMasterClock());
}

void Cx4Debugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	Cx4State& state = _cx4->GetState();
	addr = (state.Cache.Address[state.Cache.Page] + (state.PC * 2)) & 0xFFFFFF;

	AddressInfo addressInfo = _cx4->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::Cx4Memory);
	InstructionProgress.LastMemOperation = operation;

	if(addressInfo.Type == MemoryType::SnesPrgRom) {
		_codeDataLogger->SetData<SnesCdlFlags::Cx4>(addressInfo.Address);
	}
	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation, addressInfo);
	}
	_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());

	_debugger->ProcessBreakConditions(CpuType::Cx4, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void Cx4Debugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _cx4->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::Cx4Memory);
	InstructionProgress.LastMemOperation = operation;
	_debugger->ProcessBreakConditions(CpuType::Cx4, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetMasterClock());
	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation, addressInfo);
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
			step.BreakAddress = _callstackManager->GetReturnAddress();
			step.BreakStackPointer = _callstackManager->GetReturnStackPointer();
			break;

		case StepType::StepOver:
			if(Cx4DisUtils::IsJumpToSub(_prevOpCode)) {
				step.BreakAddress = _prevProgramCounter + Cx4DisUtils::GetOpSize();
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

void Cx4Debugger::SetProgramCounter(uint32_t addr, bool updateDebuggerOnly)
{
	Cx4State& state = _cx4->GetState();
	if(addr >= state.Cache.Base) {
		MemoryMappings* mappings = _cx4->GetMemoryMappings();
		uint16_t opCode = mappings->PeekWord(addr);
		_prevProgramCounter = addr;
		_prevOpCode = (opCode >> 8) & 0xFC;
		_prevStackPointer = _cx4->GetState().SP;

		addr -= state.Cache.Base;
		if(!updateDebuggerOnly) {
			state.PB = (addr & 0xFFFE00) >> 9;
			state.PC = (addr & 0x1FF) >> 1;
		}
	}
}

uint32_t Cx4Debugger::GetProgramCounter(bool getInstPc)
{
	Cx4State& state = _cx4->GetState();
	return getInstPc ? _prevProgramCounter : ((state.Cache.Address[state.Cache.Page] + (state.PC * 2)) & 0xFFFFFF);
}

uint64_t Cx4Debugger::GetCpuCycleCount(bool forProfiler)
{
	return _cx4->GetState().CycleCount;
}

DebuggerFeatures Cx4Debugger::GetSupportedFeatures()
{
	DebuggerFeatures features = {};
	features.ChangeProgramCounter = AllowChangeProgramCounter;
	features.CallStack = true;
	features.StepOut = true;
	features.StepOver = true;
	return features;
}

BreakpointManager* Cx4Debugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

CallstackManager* Cx4Debugger::GetCallstackManager()
{
	return _callstackManager.get();
}

IAssembler* Cx4Debugger::GetAssembler()
{
	throw std::runtime_error("Assembler not supported for CX4");
}

BaseEventManager* Cx4Debugger::GetEventManager()
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
