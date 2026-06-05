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

optional<uint32_t> Cx4Debugger::GetRegisterValue(RegisterType registerType)
{
	Cx4State& s = _cx4->GetState();
	switch(registerType) {
		default: return {};
		case RegisterType::R0: return s.Regs[0];
		case RegisterType::R1: return s.Regs[1];
		case RegisterType::R2: return s.Regs[2];
		case RegisterType::R3: return s.Regs[3];
		case RegisterType::R4: return s.Regs[4];
		case RegisterType::R5: return s.Regs[5];
		case RegisterType::R6: return s.Regs[6];
		case RegisterType::R7: return s.Regs[7];
		case RegisterType::R8: return s.Regs[8];
		case RegisterType::R9: return s.Regs[9];
		case RegisterType::R10: return s.Regs[10];
		case RegisterType::R11: return s.Regs[11];
		case RegisterType::R12: return s.Regs[12];
		case RegisterType::R13: return s.Regs[13];
		case RegisterType::R14: return s.Regs[14];
		case RegisterType::R15: return s.Regs[15];
		case RegisterType::Pb: return s.PB;
		case RegisterType::Pc: return s.PC;
		case RegisterType::A: return s.A;
		case RegisterType::P: return s.P;
		case RegisterType::Sp: return s.SP;
		case RegisterType::Ml: return (uint32_t)(s.Mult & 0xFFFFFF);
		case RegisterType::Mh: return (uint32_t)(s.Mult >> 24);
		case RegisterType::Mdr: return s.MemoryDataReg;
		case RegisterType::Mar: return s.MemoryAddressReg;
		case RegisterType::Dpr: return s.DataPointerReg;
	}
}

bool Cx4Debugger::SetRegisterValue(RegisterType registerType, uint32_t value)
{
	Cx4State& s = _cx4->GetState();
	switch(registerType) {
		default: return false;
		case RegisterType::R0: s.Regs[0] = value & 0xFFFFFF; break;
		case RegisterType::R1: s.Regs[1] = value & 0xFFFFFF; break;
		case RegisterType::R2: s.Regs[2] = value & 0xFFFFFF; break;
		case RegisterType::R3: s.Regs[3] = value & 0xFFFFFF; break;
		case RegisterType::R4: s.Regs[4] = value & 0xFFFFFF; break;
		case RegisterType::R5: s.Regs[5] = value & 0xFFFFFF; break;
		case RegisterType::R6: s.Regs[6] = value & 0xFFFFFF; break;
		case RegisterType::R7: s.Regs[7] = value & 0xFFFFFF; break;
		case RegisterType::R8: s.Regs[8] = value & 0xFFFFFF; break;
		case RegisterType::R9: s.Regs[9] = value & 0xFFFFFF; break;
		case RegisterType::R10: s.Regs[10] = value & 0xFFFFFF; break;
		case RegisterType::R11: s.Regs[11] = value & 0xFFFFFF; break;
		case RegisterType::R12: s.Regs[12] = value & 0xFFFFFF; break;
		case RegisterType::R13: s.Regs[13] = value & 0xFFFFFF; break;
		case RegisterType::R14: s.Regs[14] = value & 0xFFFFFF; break;
		case RegisterType::R15: s.Regs[15] = value & 0xFFFFFF; break;
		case RegisterType::Pb: s.PB = (uint16_t)value; break;
		case RegisterType::Pc: s.PC = (uint8_t)value; break;
		case RegisterType::A: s.A = value & 0xFFFFFF; break;
		case RegisterType::P: s.P = (uint16_t)value; break;
		case RegisterType::Sp: s.SP = (uint8_t)value; break;
		case RegisterType::Ml: s.Mult = (s.Mult & 0xFFFFFF000000) | (value & 0xFFFFFF); break;
		case RegisterType::Mh: s.Mult = (s.Mult & 0xFFFFFF) | ((uint64_t)(value & 0xFFFFFF) << 24); break;
		case RegisterType::Mdr: s.MemoryDataReg = value & 0xFFFFFF; break;
		case RegisterType::Mar: s.MemoryAddressReg = value & 0xFFFFFF; break;
		case RegisterType::Dpr: s.DataPointerReg = value & 0xFFFFFF; break;
	}
	return true;
}

BaseState& Cx4Debugger::GetState()
{
	return _cx4->GetState();
}

ITraceLogger* Cx4Debugger::GetTraceLogger()
{
	return _traceLogger.get();
}
