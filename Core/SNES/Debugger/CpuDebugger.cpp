#include "stdafx.h"
#include "SNES/Cpu.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/MemoryManager.h"
#include "SNES/BaseCartridge.h"
#include "SNES/Console.h"
#include "SNES/Spc.h"
#include "SNES/Ppu.h"
#include "SNES/MemoryMappings.h"
#include "SNES/Debugger/Assembler.h"
#include "SNES/Debugger/CpuDebugger.h"
#include "SNES/Debugger/EventManager.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
#include "Debugger/TraceLogger.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/CodeDataLogger.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Debugger/ScriptManager.h"
#include "Debugger/Debugger.h"
#include "Debugger/CodeDataLogger.h"
#include "Shared/SettingTypes.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "MemoryOperationType.h"

CpuDebugger::CpuDebugger(Debugger* debugger, CpuType cpuType)
{
	_cpuType = cpuType;

	_debugger = debugger;
	Console* console = (Console*)debugger->GetConsole();
	_traceLogger = debugger->GetTraceLogger().get();
	_disassembler = debugger->GetDisassembler().get();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter().get();
	_cpu = console->GetCpu().get();
	_sa1 = console->GetCartridge()->GetSa1();
	_settings = debugger->GetEmulator()->GetSettings();
	_memoryManager = console->GetMemoryManager().get();
	_cart = console->GetCartridge().get();
	_spc = console->GetSpc().get();
	_ppu = console->GetPpu().get();
	
	if(cpuType == CpuType::Sa1) {
		_codeDataLogger = _debugger->GetCodeDataLogger(CpuType::Cpu);
	} else {
		_codeDataLogger.reset(new CodeDataLogger(console->GetCartridge()->DebugGetPrgRomSize(), CpuType::Cpu));
	}

	_eventManager.reset(new EventManager(debugger, _cpu, console->GetPpu().get(), _memoryManager, console->GetDmaController().get()));
	_callstackManager.reset(new CallstackManager(debugger));
	_breakpointManager.reset(new BreakpointManager(debugger, cpuType, _eventManager.get()));
	_step.reset(new StepRequest());
	_assembler.reset(new Assembler(_debugger->GetLabelManager()));

	if(GetCpuState().PC == 0) {
		//Enable breaking on uninit reads when debugger is opened at power on
		_enableBreakOnUninitRead = true;
	}
}

void CpuDebugger::Reset()
{
	_enableBreakOnUninitRead = true;
	_callstackManager.reset(new CallstackManager(_debugger));
	_prevOpCode = 0xFF;
}

void CpuDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = GetMemoryMappings().GetAbsoluteAddress(addr);
	MemoryOperationInfo operation = { addr, value, type };
	CpuState state = GetCpuState();
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
			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, state.PS, _cpuType);
			_traceLogger->Log(_cpuType, state, disInfo);
		}

		uint32_t pc = (state.K << 16) | state.PC;
		if(_prevOpCode == 0x20 || _prevOpCode == 0x22 || _prevOpCode == 0xFC) {
			//JSR, JSL
			uint8_t opSize = DisassemblyInfo::GetOpSize(_prevOpCode, state.PS, _cpuType);
			uint32_t returnPc = (_prevProgramCounter & 0xFF0000) | (((_prevProgramCounter & 0xFFFF) + opSize) & 0xFFFF);
			AddressInfo srcAddress = GetMemoryMappings().GetAbsoluteAddress(_prevProgramCounter);
			AddressInfo retAddress = GetMemoryMappings().GetAbsoluteAddress(returnPc);
			_callstackManager->Push(srcAddress, _prevProgramCounter, addressInfo, pc, retAddress, returnPc, StackFrameFlags::None);
		} else if(_prevOpCode == 0x60 || _prevOpCode == 0x6B || _prevOpCode == 0x40) {
			//RTS, RTL, RTI
			_callstackManager->Pop(addressInfo, pc);
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
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
	} else if(type == MemoryOperationType::ExecOperand) {
		if(addressInfo.Type == SnesMemoryType::PrgRom && addressInfo.Address >= 0) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Code | (state.PS & (CdlFlags::IndexMode8 | CdlFlags::MemoryMode8)));
		}
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
	} else {
		if(addressInfo.Type == SnesMemoryType::PrgRom && addressInfo.Address >= 0) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Data | (state.PS & (CdlFlags::IndexMode8 | CdlFlags::MemoryMode8)));
		}

		if(_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock())) {
			//Memory access was a read on an uninitialized memory address
			if(_enableBreakOnUninitRead) {
				if(_memoryAccessCounter->GetReadCount(addressInfo) == 1) {
					//Only warn the first time
					_debugger->Log(string(_cpuType == CpuType::Sa1 ? "[SA1]" : "[CPU]") + " Uninitialized memory read: $" + HexUtilities::ToHex24(addr));
				}
				if(_settings->CheckDebuggerFlag(DebuggerFlags::CpuDebuggerEnabled) && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnUninitRead)) {
					breakSource = BreakSource::BreakOnUninitMemoryRead;
					_step->StepCount = 0;
				}
			}
		}
	}

	if(IsRegister(addr)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	_debugger->ProcessBreakConditions(_step->StepCount == 0, _breakpointManager.get(), operation, addressInfo, breakSource);
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

	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetMasterClock());

	_debugger->ProcessBreakConditions(false, _breakpointManager.get(), operation, addressInfo);
}

void CpuDebugger::Run()
{
	_step.reset(new StepRequest());
}

void CpuDebugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step;
	if((type == StepType::StepOver || type == StepType::StepOut || type == StepType::Step) && GetCpuState().StopState == CpuStopState::Stopped) {
		//If STP was called, the CPU isn't running anymore - use the PPU to break execution instead (useful for test roms that end with STP)
		step.PpuStepCount = 1;
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

			case StepType::PpuStep: step.PpuStepCount = stepCount; break;
			case StepType::SpecificScanline: step.BreakScanline = stepCount; break;
		}
	}
	_step.reset(new StepRequest(step));
}

void CpuDebugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	AddressInfo src = GetMemoryMappings().GetAbsoluteAddress(_prevProgramCounter);
	AddressInfo ret = GetMemoryMappings().GetAbsoluteAddress(originalPc);
	AddressInfo dest = GetMemoryMappings().GetAbsoluteAddress(currentPc);
	_callstackManager->Push(src, _prevProgramCounter, dest, currentPc, ret, originalPc, forNmi ? StackFrameFlags::Nmi : StackFrameFlags::Irq);
	_eventManager->AddEvent(forNmi ? DebugEventType::Nmi : DebugEventType::Irq);
}

void CpuDebugger::ProcessPpuCycle(uint16_t &scanline, uint16_t &cycle)
{
	if(_cpuType == CpuType::Cpu) {
		scanline = _ppu->GetScanline();
		cycle = _memoryManager->GetHClock();

		//Catch up SPC/DSP as needed (if we're tracing or debugging those particular CPUs)
		if(_traceLogger->IsCpuLogged(CpuType::Spc) || _settings->CheckDebuggerFlag(DebuggerFlags::SpcDebuggerEnabled)) {
			_spc->Run();
		} else if(_traceLogger->IsCpuLogged(CpuType::NecDsp)) {
			_cart->RunCoprocessors();
		}
	}

	if(_step->PpuStepCount > 0) {
		_step->PpuStepCount--;
		if(_step->PpuStepCount == 0) {
			_debugger->SleepUntilResume(BreakSource::PpuStep);
		}
	}

	if(cycle == 0 && scanline == _step->BreakScanline) {
		_step->BreakScanline = -1;
		_debugger->SleepUntilResume(BreakSource::PpuStep);
	}
}

MemoryMappings& CpuDebugger::GetMemoryMappings()
{
	if(_cpuType == CpuType::Cpu) {
		return *_memoryManager->GetMemoryMappings();
	} else {
		return *_sa1->GetMemoryMappings();
	}
}

CpuState& CpuDebugger::GetCpuState()
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

shared_ptr<IAssembler> CpuDebugger::GetAssembler()
{
	return _assembler;
}

shared_ptr<IEventManager> CpuDebugger::GetEventManager()
{
	return _eventManager;
}

shared_ptr<CodeDataLogger> CpuDebugger::GetCodeDataLogger()
{
	return _codeDataLogger;
}

BaseState& CpuDebugger::GetState()
{
	return GetCpuState();
}
