#include "pch.h"
#include "SNES/Debugger/GsuDebugger.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/SnesConsole.h"
#include "SNES/BaseCartridge.h"
#include "SNES/Coprocessors/GSU/Gsu.h"
#include "SNES/Debugger/SnesCodeDataLogger.h"
#include "SNES/Debugger/TraceLogger/GsuTraceLogger.h"
#include "Debugger/CdlManager.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Debugger/Debugger.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/CodeDataLogger.h"
#include "Shared/MemoryOperationType.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"

GsuDebugger::GsuDebugger(Debugger* debugger) : IDebugger(debugger->GetEmulator())
{
	SnesConsole* console = (SnesConsole*)debugger->GetConsole();

	_debugger = debugger;
	_codeDataLogger = (SnesCodeDataLogger*)debugger->GetCdlManager()->GetCodeDataLogger(MemoryType::SnesPrgRom);
	_disassembler = debugger->GetDisassembler();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter();
	_gsu = console->GetCartridge()->GetGsu();
	_memoryManager = console->GetMemoryManager();
	_settings = debugger->GetEmulator()->GetSettings();
	
	_traceLogger.reset(new GsuTraceLogger(debugger, this, console->GetPpu(), _memoryManager));

	_breakpointManager.reset(new BreakpointManager(debugger, this, CpuType::Gsu, debugger->GetEventManager(CpuType::Snes)));
	_step.reset(new StepRequest());
}

void GsuDebugger::Reset()
{
	_prevOpCode = 0xFF;
}

void GsuDebugger::ProcessInstruction()
{
	GsuState& state = _gsu->GetState();
	uint32_t addr = _gsu->DebugGetProgramCounter();
	AddressInfo addressInfo = _gsu->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, state.ProgramReadBuffer, MemoryOperationType::ExecOpCode, MemoryType::GsuMemory);
	InstructionProgress.LastMemOperation = operation;
	InstructionProgress.StartCycle = state.CycleCount;

	if(addressInfo.Type == MemoryType::SnesPrgRom) {
		_codeDataLogger->SetCode<SnesCdlFlags::Gsu>(addressInfo.Address);
	}

	if(_settings->CheckDebuggerFlag(DebuggerFlags::GsuDebuggerEnabled)) {
		_disassembler->BuildCache(addressInfo, state.SFR.GetFlagsHigh() & 0x13, CpuType::Gsu);
	}

	_prevOpCode = state.ProgramReadBuffer;
	_prevProgramCounter = addr;

	_step->ProcessCpuExec();

	_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
	_debugger->ProcessBreakConditions(CpuType::Gsu, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void GsuDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _gsu->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::GsuMemory);
	InstructionProgress.LastMemOperation = operation;

	if(type == MemoryOperationType::ExecOpCode) {
		if(_traceLogger->IsEnabled()) {
			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::Gsu);
			_traceLogger->Log(_gsu->GetState(), disInfo, operation, addressInfo);
		}
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
	} else if(type == MemoryOperationType::ExecOperand) {
		if(addressInfo.Type == MemoryType::SnesPrgRom) {
			_codeDataLogger->SetData<SnesCdlFlags::Gsu>(addressInfo.Address);
		}
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
	} else {
		if(addressInfo.Type == MemoryType::SnesPrgRom) {
			_codeDataLogger->SetData<SnesCdlFlags::Gsu>(addressInfo.Address);
		}
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation, addressInfo);
		}
		_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());
		_debugger->ProcessBreakConditions(CpuType::Gsu, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	}
}

void GsuDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _gsu->GetMemoryMappings()->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::GsuMemory);
	InstructionProgress.LastMemOperation = operation;

	_debugger->ProcessBreakConditions(CpuType::Gsu, *_step.get(), _breakpointManager.get(), operation, addressInfo);

	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation, addressInfo);
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

void GsuDebugger::SetProgramCounter(uint32_t addr, bool updateDebuggerOnly)
{
	if(!updateDebuggerOnly) {
		_gsu->DebugSetProgramCounter(addr);
	}

	GsuState& state = _gsu->GetState();
	_prevOpCode = state.ProgramReadBuffer;
	_prevProgramCounter = addr;
}

uint32_t GsuDebugger::GetProgramCounter(bool getInstPc)
{
	GsuState& state = _gsu->GetState();
	return getInstPc ? _prevProgramCounter : ((state.ProgramBank << 16) | state.R[15]);
}

uint64_t GsuDebugger::GetCpuCycleCount(bool forProfiler)
{
	return _gsu->GetState().CycleCount;
}

DebuggerFeatures GsuDebugger::GetSupportedFeatures()
{
	DebuggerFeatures features = {};
	features.ChangeProgramCounter = AllowChangeProgramCounter;
	return features;
}

BreakpointManager* GsuDebugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

CallstackManager* GsuDebugger::GetCallstackManager()
{
	return nullptr;
}

IAssembler* GsuDebugger::GetAssembler()
{
	throw std::runtime_error("Assembler not supported for GSU");
}

BaseEventManager* GsuDebugger::GetEventManager()
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
