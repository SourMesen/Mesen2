#include "stdafx.h"
#include "CpuDebugger.h"
#include "DebugTypes.h"
#include "SettingTypes.h"
#include "DisassemblyInfo.h"
#include "Disassembler.h"
#include "Cpu.h"
#include "Sa1.h"
#include "TraceLogger.h"
#include "CallstackManager.h"
#include "BreakpointManager.h"
#include "MemoryManager.h"
#include "CodeDataLogger.h"
#include "EmuSettings.h"
#include "EventManager.h"
#include "ScriptManager.h"
#include "MemoryMappings.h"
#include "Debugger.h"
#include "BaseCartridge.h"
#include "Console.h"
#include "MemoryAccessCounter.h"
#include "ExpressionEvaluator.h"

CpuDebugger::CpuDebugger(Debugger* debugger, CpuType cpuType)
{
	_cpuType = cpuType;

	_debugger = debugger;
	_traceLogger = debugger->GetTraceLogger().get();
	_disassembler = debugger->GetDisassembler().get();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter().get();
	_cpu = debugger->GetConsole()->GetCpu().get();
	_sa1 = debugger->GetConsole()->GetCartridge()->GetSa1();
	_codeDataLogger = debugger->GetCodeDataLogger().get();
	_settings = debugger->GetConsole()->GetSettings().get();
	_eventManager = debugger->GetEventManager().get();
	_scriptManager = debugger->GetScriptManager().get();
	_memoryManager = debugger->GetConsole()->GetMemoryManager().get();

	_callstackManager.reset(new CallstackManager(debugger));
	_breakpointManager.reset(new BreakpointManager(debugger, cpuType));
	_step.reset(new StepRequest());

	if(GetState().PC == 0) {
		//Enable breaking on uninit reads when debugger is opened at power on
		_enableBreakOnUninitRead = true;
	}
}

void CpuDebugger::Reset()
{
	_enableBreakOnUninitRead = true;
	_prevOpCode = 0xFF;
}

void CpuDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = GetMemoryMappings().GetAbsoluteAddress(addr);
	MemoryOperationInfo operation = { addr, value, type };
	CpuState state = GetState();
	BreakSource breakSource = BreakSource::Unspecified;

	if(type == MemoryOperationType::ExecOpCode) {
		bool needDisassemble = _traceLogger->IsCpuLogged(_cpuType) || _settings->CheckDebuggerFlag(_cpuType == CpuType::Cpu ? DebuggerFlags::CpuDebuggerEnabled : DebuggerFlags::Sa1DebuggerEnabled);
		if(addressInfo.Address >= 0) {
			if(addressInfo.Type == SnesMemoryType::PrgRom) {
				uint8_t flags = CdlFlags::Code | (state.PS & (CdlFlags::IndexMode8 | CdlFlags::MemoryMode8));
				if(_prevOpCode == 0x20 || _prevOpCode == 0x22 || _prevOpCode == 0xFC) {
					flags |= CdlFlags::SubEntryPoint;
				}
				_codeDataLogger->SetFlags(addressInfo.Address, flags);
			}
			if(needDisassemble) {
				_disassembler->BuildCache(addressInfo, state.PS & (ProcFlags::IndexMode8 | ProcFlags::MemoryMode8), _cpuType);
			}
		}

		if(_traceLogger->IsCpuLogged(_cpuType)) {
			DebugState debugState;
			_debugger->GetState(debugState, true);

			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo);
			_traceLogger->Log(_cpuType, debugState, disInfo);
		}

		uint32_t pc = (state.K << 16) | state.PC;
		if(_prevOpCode == 0x20 || _prevOpCode == 0x22 || _prevOpCode == 0xFC) {
			//JSR, JSL
			uint8_t opSize = DisassemblyInfo::GetOpSize(_prevOpCode, state.PS, _cpuType);
			uint32_t returnPc = (_prevProgramCounter & 0xFF0000) | (((_prevProgramCounter & 0xFFFF) + opSize) & 0xFFFF);
			_callstackManager->Push(_prevProgramCounter, pc, returnPc, StackFrameFlags::None);
		} else if(_prevOpCode == 0x60 || _prevOpCode == 0x6B || _prevOpCode == 0x40) {
			//RTS, RTL, RTI
			_callstackManager->Pop(pc);
		}

		if(_step->BreakAddress == (int32_t)pc && (_prevOpCode == 0x60 || _prevOpCode == 0x40 || _prevOpCode == 0x6B || _prevOpCode == 0x44 || _prevOpCode == 0x54)) {
			//RTS/RTL/RTI found, if we're on the expected return address, break immediately (for step over/step out)
			_step->StepCount = 0;
		}

		_prevOpCode = value;
		_prevProgramCounter = pc;

		if(_step->StepCount > 0) {
			_step->StepCount--;
		}

		if(_settings->CheckDebuggerFlag(DebuggerFlags::CpuDebuggerEnabled)) {
			if(value == 0x00 || value == 0x02 || value == 0x42 || value == 0xDB) {
				//Break on BRK/STP/WDM/COP
				if(value == 0x00 && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnBrk)) {
					breakSource = BreakSource::BreakOnBrk;
					_step->StepCount = 0;
				} else if(value == 0x02 && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnCop)) {
					breakSource = BreakSource::BreakOnCop;
					_step->StepCount = 0;
				} else if(value == 0x42 && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnWdm)) {
					breakSource = BreakSource::BreakOnWdm;
					_step->StepCount = 0;
				} else if(value == 0xDB && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnStp)) {
					breakSource = BreakSource::BreakOnStp;
					_step->StepCount = 0;
				}
			}
		}
	} else if(type == MemoryOperationType::ExecOperand) {
		if(addressInfo.Type == SnesMemoryType::PrgRom && addressInfo.Address >= 0) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Code | (state.PS & (CdlFlags::IndexMode8 | CdlFlags::MemoryMode8)));
		}
	} else {
		if(addressInfo.Type == SnesMemoryType::PrgRom && addressInfo.Address >= 0) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Data | (state.PS & (CdlFlags::IndexMode8 | CdlFlags::MemoryMode8)));
		}
	}

	if(_memoryAccessCounter->ProcessMemoryAccess(addressInfo, type, _memoryManager->GetMasterClock())) {
		//Memory access was a read on an uninitialized memory address
		if(_enableBreakOnUninitRead && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnUninitRead)) {
			breakSource = BreakSource::BreakOnUninitMemoryRead;
			_step->StepCount = 0;
		}
	}

	if(IsRegister(addr)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	_debugger->ProcessBreakConditions(_step->StepCount == 0, _breakpointManager.get(), operation, addressInfo, breakSource);

	_scriptManager->ProcessMemoryOperation(addr, value, type);
}

void CpuDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = GetMemoryMappings().GetAbsoluteAddress(addr);
	MemoryOperationInfo operation = { addr, value, type };
	if(addressInfo.Address >= 0 && (addressInfo.Type == SnesMemoryType::WorkRam || addressInfo.Type == SnesMemoryType::SaveRam)) {
		_disassembler->InvalidateCache(addressInfo, _cpuType);
	}

	if(IsRegister(addr)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, type, _memoryManager->GetMasterClock());

	_debugger->ProcessBreakConditions(false, _breakpointManager.get(), operation, addressInfo);

	_scriptManager->ProcessMemoryOperation(addr, value, type);
}

void CpuDebugger::Run()
{
	_step.reset(new StepRequest());
}

void CpuDebugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step;
	if((type == StepType::StepOver || type == StepType::StepOut || type == StepType::Step) && GetState().StopState == CpuStopState::Stopped) {
		//If STP was called, the CPU isn't running anymore - use the PPU to break execution instead (useful for test roms that end with STP)
		_debugger->Step(_cpuType, 1, StepType::PpuStep);
	} else {
		switch(type) {
			case StepType::Step: step.StepCount = stepCount; break;
			case StepType::StepOut: step.BreakAddress = _callstackManager->GetReturnAddress(); break;
			case StepType::StepOver:
				if(_prevOpCode == 0x20 || _prevOpCode == 0x22 || _prevOpCode == 0xFC || _prevOpCode == 0x00 || _prevOpCode == 0x02 || _prevOpCode == 0x44 || _prevOpCode == 0x54) {
					//JSR, JSL, BRK, COP, MVP, MVN
					step.BreakAddress = (_prevProgramCounter & 0xFF0000) | (((_prevProgramCounter & 0xFFFF) + DisassemblyInfo::GetOpSize(_prevOpCode, 0, _cpuType)) & 0xFFFF);
				} else {
					//For any other instruction, step over is the same as step into
					step.StepCount = 1;
				}
				break;

			case StepType::SpecificScanline:
			case StepType::PpuStep:
				break;
		}
	}
	_step.reset(new StepRequest(step));
}

void CpuDebugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	_callstackManager->Push(_prevProgramCounter, currentPc, originalPc, forNmi ? StackFrameFlags::Nmi : StackFrameFlags::Irq);
	_eventManager->AddEvent(forNmi ? DebugEventType::Nmi : DebugEventType::Irq);
}

MemoryMappings& CpuDebugger::GetMemoryMappings()
{
	if(_cpuType == CpuType::Cpu) {
		return *_memoryManager->GetMemoryMappings();
	} else {
		return *_sa1->GetMemoryMappings();
	}
}

CpuState CpuDebugger::GetState()
{
	if(_cpuType == CpuType::Cpu) {
		return _cpu->GetState();
	} else {
		return _sa1->GetCpuState();
	}
}

bool CpuDebugger::IsRegister(uint32_t addr)
{
	return _cpuType == CpuType::Cpu && _memoryManager->IsRegister(addr);
}

shared_ptr<CallstackManager> CpuDebugger::GetCallstackManager()
{
	return _callstackManager;
}

BreakpointManager* CpuDebugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}