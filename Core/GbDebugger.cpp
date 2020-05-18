#include "stdafx.h"
#include "GbDebugger.h"
#include "DisassemblyInfo.h"
#include "Disassembler.h"
#include "Gameboy.h"
#include "TraceLogger.h"
#include "CallstackManager.h"
#include "BreakpointManager.h"
#include "MemoryManager.h"
#include "Debugger.h"
#include "Console.h"
#include "MemoryAccessCounter.h"
#include "ExpressionEvaluator.h"
#include "EmuSettings.h"
#include "BaseCartridge.h"
#include "GameboyDisUtils.h"

GbDebugger::GbDebugger(Debugger* debugger)
{
	_debugger = debugger;
	_traceLogger = debugger->GetTraceLogger().get();
	_disassembler = debugger->GetDisassembler().get();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter().get();
	_gameboy = debugger->GetConsole()->GetCartridge()->GetGameboy();
	_memoryManager = debugger->GetConsole()->GetMemoryManager().get();
	_settings = debugger->GetConsole()->GetSettings().get();

	_callstackManager.reset(new CallstackManager(debugger));
	_breakpointManager.reset(new BreakpointManager(debugger, CpuType::Gameboy));
	_step.reset(new StepRequest());
}

void GbDebugger::Reset()
{
	_callstackManager.reset(new CallstackManager(_debugger));
	_prevOpCode = 0;
}

void GbDebugger::ProcessRead(uint16_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _gameboy->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation { addr, value, type };
	BreakSource breakSource = BreakSource::Unspecified;

	if(type == MemoryOperationType::ExecOpCode) {
		GbCpuState gbState = _gameboy->GetState().Cpu;

		if(_traceLogger->IsCpuLogged(CpuType::Gameboy) || _settings->CheckDebuggerFlag(DebuggerFlags::GbDebuggerEnabled)) {
			if(addressInfo.Address >= 0) {
				_disassembler->BuildCache(addressInfo, 0, CpuType::Gameboy);
			}

			if(_traceLogger->IsCpuLogged(CpuType::Gameboy)) {
				DebugState debugState;
				_debugger->GetState(debugState, true);

				DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::Gameboy);
				_traceLogger->Log(CpuType::Gameboy, debugState, disInfo);
			}
		}

		if(GameboyDisUtils::IsJumpToSub(_prevOpCode) && gbState.PC != _prevProgramCounter + GameboyDisUtils::GetOpSize(_prevOpCode)) {
			//CALL and RST, and PC doesn't match the next instruction, so the call was (probably) done
			uint8_t opSize = DisassemblyInfo::GetOpSize(_prevOpCode, 0, CpuType::Gameboy);
			uint16_t returnPc = _prevProgramCounter + opSize;
			AddressInfo src = _gameboy->GetAbsoluteAddress(_prevProgramCounter);
			AddressInfo ret = _gameboy->GetAbsoluteAddress(returnPc);
			_callstackManager->Push(src, _prevProgramCounter, addressInfo, gbState.PC, ret, returnPc, StackFrameFlags::None);
		} else if(GameboyDisUtils::IsReturnInstruction(_prevOpCode) && gbState.PC != _prevProgramCounter + GameboyDisUtils::GetOpSize(_prevOpCode)) {
			//RET used, and PC doesn't match the next instruction, so the ret was (probably) taken
			_callstackManager->Pop(addressInfo, gbState.PC);
		}

		if(_step->BreakAddress == (int32_t)gbState.PC && GameboyDisUtils::IsReturnInstruction(_prevOpCode)) {
			//RET/RETI found, if we're on the expected return address, break immediately (for step over/step out)
			_step->StepCount = 0;
		}

		_prevOpCode = value;
		_prevProgramCounter = gbState.PC;

		if(_step->StepCount > 0) {
			_step->StepCount--;
		}

		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
	} else if(type == MemoryOperationType::ExecOperand) {
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
	} else {
		_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());
	}

	_debugger->ProcessBreakConditions(_step->StepCount == 0, GetBreakpointManager(), operation, addressInfo, breakSource);
}

void GbDebugger::ProcessWrite(uint16_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _gameboy->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation { addr, value, type };
	_debugger->ProcessBreakConditions(false, GetBreakpointManager(), operation, addressInfo);

	if(addressInfo.Type == SnesMemoryType::GbWorkRam || addressInfo.Type == SnesMemoryType::GbCartRam || addressInfo.Type == SnesMemoryType::GbHighRam) {
		_disassembler->InvalidateCache(addressInfo, CpuType::Gameboy);
	}

	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetMasterClock());
}

void GbDebugger::Run()
{
	_step.reset(new StepRequest());
}

void GbDebugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step;

	switch(type) {
		case StepType::Step: step.StepCount = stepCount; break;
		case StepType::StepOut: step.BreakAddress = _callstackManager->GetReturnAddress(); break;
		case StepType::StepOver:
			if(GameboyDisUtils::IsJumpToSub(_prevOpCode)) {
				step.BreakAddress = _prevProgramCounter + DisassemblyInfo::GetOpSize(_prevOpCode, 0, CpuType::Gameboy);
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

shared_ptr<CallstackManager> GbDebugger::GetCallstackManager()
{
	return _callstackManager;
}

BreakpointManager* GbDebugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}