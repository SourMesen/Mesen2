#include "stdafx.h"
#include "SNES/SnesCpu.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/BaseCartridge.h"
#include "SNES/SnesConsole.h"
#include "SNES/Spc.h"
#include "SNES/SnesPpu.h"
#include "SNES/MemoryMappings.h"
#include "SNES/Debugger/SnesAssembler.h"
#include "SNES/Debugger/SnesDebugger.h"
#include "SNES/Debugger/SnesEventManager.h"
#include "SNES/Debugger/TraceLogger/SnesCpuTraceLogger.h"
#include "SNES/Debugger/SnesPpuTools.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
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

SnesDebugger::SnesDebugger(Debugger* debugger, CpuType cpuType)
{
	_cpuType = cpuType;

	_debugger = debugger;
	SnesConsole* console = (SnesConsole*)debugger->GetConsole();
	_console = console;
	_disassembler = debugger->GetDisassembler();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter();
	_cpu = console->GetCpu();
	_sa1 = console->GetCartridge()->GetSa1();
	_settings = debugger->GetEmulator()->GetSettings();
	_memoryManager = console->GetMemoryManager();
	_cart = console->GetCartridge();
	_spc = console->GetSpc();
	_ppu = console->GetPpu();
	_traceLogger.reset(new SnesCpuTraceLogger(debugger, cpuType, _ppu, _memoryManager));
	_ppuTools.reset(new SnesPpuTools(debugger, debugger->GetEmulator()));
	
	if(_cpuType == CpuType::Snes) {
		_memoryMappings = _memoryManager->GetMemoryMappings();
	} else {
		_memoryMappings = _sa1->GetMemoryMappings();
	}

	if(cpuType == CpuType::Snes) {
		_codeDataLogger.reset(new CodeDataLogger(SnesMemoryType::PrgRom, console->GetCartridge()->DebugGetPrgRomSize(), CpuType::Snes));
	}

	_eventManager.reset(new SnesEventManager(debugger, _cpu, console->GetPpu(), _memoryManager, console->GetDmaController()));
	_callstackManager.reset(new CallstackManager(debugger));
	_breakpointManager.reset(new BreakpointManager(debugger, cpuType, _eventManager.get()));
	_step.reset(new StepRequest());
	_assembler.reset(new SnesAssembler(_debugger->GetLabelManager()));

	if(GetCpuState().PC == 0) {
		//Enable breaking on uninit reads when debugger is opened at power on
		_enableBreakOnUninitRead = true;
	}

	_debuggerEnabledFlag = _cpuType == CpuType::Snes ? DebuggerFlags::CpuDebuggerEnabled : DebuggerFlags::Sa1DebuggerEnabled;
}

void SnesDebugger::Init()
{
	_spcTraceLogger = _debugger->GetTraceLogger(CpuType::Spc);
	_dspTraceLogger = _debugger->GetTraceLogger(CpuType::NecDsp);
}

void SnesDebugger::Reset()
{
	_enableBreakOnUninitRead = true;
	_callstackManager.reset(new CallstackManager(_debugger));
	_prevOpCode = 0xFF;
}

void SnesDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _memoryMappings->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, SnesMemoryType::CpuMemory);
	SnesCpuState& state = GetCpuState();
	BreakSource breakSource = BreakSource::Unspecified;

	if(IsRegister(addr)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	if(type == MemoryOperationType::ExecOpCode) {
		if(addressInfo.Address >= 0) {
			uint8_t cpuFlags = state.PS & (ProcFlags::IndexMode8 | ProcFlags::MemoryMode8);
			if(addressInfo.Type == SnesMemoryType::PrgRom) {
				uint8_t flags = CdlFlags::Code | cpuFlags;
				if(_prevOpCode == 0x20 || _prevOpCode == 0x22 || _prevOpCode == 0xFC) {
					flags |= CdlFlags::SubEntryPoint;
				}
				_codeDataLogger->SetFlags(addressInfo.Address, flags);
			}
			if(_traceLogger->IsEnabled() || _settings->CheckDebuggerFlag(_debuggerEnabledFlag)) {
				_disassembler->BuildCache(addressInfo, cpuFlags, _cpuType);
			}
		}

		if(_traceLogger->IsEnabled()) {
			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, state.PS, _cpuType);
			_traceLogger->Log(state, disInfo, operation);
		}

		uint32_t pc = (state.K << 16) | state.PC;
		if(_prevOpCode == 0x20 || _prevOpCode == 0x22 || _prevOpCode == 0xFC) {
			//JSR, JSL
			uint8_t opSize = DisassemblyInfo::GetOpSize(_prevOpCode, state.PS, _cpuType);
			uint32_t returnPc = (_prevProgramCounter & 0xFF0000) | (((_prevProgramCounter & 0xFFFF) + opSize) & 0xFFFF);
			AddressInfo srcAddress = _memoryMappings->GetAbsoluteAddress(_prevProgramCounter);
			AddressInfo retAddress = _memoryMappings->GetAbsoluteAddress(returnPc);
			_callstackManager->Push(srcAddress, _prevProgramCounter, addressInfo, pc, retAddress, returnPc, StackFrameFlags::None);
		} else if(_prevOpCode == 0x60 || _prevOpCode == 0x6B || _prevOpCode == 0x40) {
			//RTS, RTL, RTI
			_callstackManager->Pop(addressInfo, pc);
		}

		if(_step->BreakAddress == (int32_t)pc && (_prevOpCode == 0x60 || _prevOpCode == 0x40 || _prevOpCode == 0x6B || _prevOpCode == 0x44 || _prevOpCode == 0x54)) {
			//RTS/RTL/RTI found, if we're on the expected return address, break immediately (for step over/step out)
			_step->StepCount = 0;
			breakSource = BreakSource::CpuStep;
		}

		_prevOpCode = value;
		_prevProgramCounter = pc;

		_step->ProcessCpuExec(&breakSource);

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
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
	} else {
		if(addressInfo.Type == SnesMemoryType::PrgRom && addressInfo.Address >= 0) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Data | (state.PS & (CdlFlags::IndexMode8 | CdlFlags::MemoryMode8)));
		}
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
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

	_debugger->ProcessBreakConditions(_step->StepCount == 0, _breakpointManager.get(), operation, addressInfo, breakSource);
}

void SnesDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _memoryMappings->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, SnesMemoryType::CpuMemory);
	if(addressInfo.Address >= 0 && (addressInfo.Type == SnesMemoryType::WorkRam || addressInfo.Type == SnesMemoryType::SaveRam)) {
		_disassembler->InvalidateCache(addressInfo, _cpuType);
	}

	if(IsRegister(addr)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation);
	}

	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetMasterClock());

	_debugger->ProcessBreakConditions(false, _breakpointManager.get(), operation, addressInfo);
}

void SnesDebugger::Run()
{
	_step.reset(new StepRequest());
}

void SnesDebugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step;
	if((type == StepType::StepOver || type == StepType::StepOut || type == StepType::Step) && GetCpuState().StopState == SnesCpuStopState::Stopped) {
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
			case StepType::PpuScanline: step.PpuStepCount = 341; break;
			case StepType::PpuFrame: step.PpuStepCount = 341 * (_ppu->GetVblankEndScanline() + 1); break;
			case StepType::SpecificScanline: step.BreakScanline = stepCount; break;
		}
	}
	_step.reset(new StepRequest(step));
}

void SnesDebugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	AddressInfo src = _memoryMappings->GetAbsoluteAddress(_prevProgramCounter);
	AddressInfo ret = _memoryMappings->GetAbsoluteAddress(originalPc);
	AddressInfo dest = _memoryMappings->GetAbsoluteAddress(currentPc);
	_callstackManager->Push(src, _prevProgramCounter, dest, currentPc, ret, originalPc, forNmi ? StackFrameFlags::Nmi : StackFrameFlags::Irq);
	_eventManager->AddEvent(forNmi ? DebugEventType::Nmi : DebugEventType::Irq);
}

void SnesDebugger::ProcessPpuRead(uint16_t addr, uint8_t value, SnesMemoryType memoryType)
{
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Read, memoryType);
	AddressInfo addressInfo { addr, memoryType };
	_debugger->ProcessBreakConditions(false, _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryRead(addressInfo, _console->GetMasterClock());
}

void SnesDebugger::ProcessPpuWrite(uint16_t addr, uint8_t value, SnesMemoryType memoryType)
{
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Write, memoryType);
	AddressInfo addressInfo { addr, memoryType };
	_debugger->ProcessBreakConditions(false, _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _console->GetMasterClock());
}

void SnesDebugger::ProcessPpuCycle()
{
	//Catch up SPC/DSP as needed (if we're tracing or debugging those particular CPUs)
	if(_spcTraceLogger->IsEnabled()) {
		_spc->Run();
	}
	if(_dspTraceLogger && _dspTraceLogger->IsEnabled()) {
		_cart->RunCoprocessors();
	}

	if(_ppuTools->HasOpenedViewer()) {
		_ppuTools->UpdateViewers(_ppu->GetScanline(), _ppu->GetCycle());
	}

	if(_step->HasRequest) {
		if(_step->HasScanlineBreakRequest() && _ppu->GetScanline() == _step->BreakScanline && _memoryManager->GetHClock() == 0) {
			_debugger->SleepUntilResume(BreakSource::PpuStep);
		} else if(_step->PpuStepCount > 0) {
			_step->PpuStepCount--;
			if(_step->PpuStepCount == 0) {
				_debugger->SleepUntilResume(BreakSource::PpuStep);
			}
		}
	}
}

SnesCpuState& SnesDebugger::GetCpuState()
{
	if(_cpuType == CpuType::Snes) {
		return _cpu->GetState();
	} else {
		return _sa1->GetCpuState();
	}
}

bool SnesDebugger::IsRegister(uint32_t addr)
{
	return _cpuType == CpuType::Snes && _memoryManager->IsRegister(addr);
}

CallstackManager* SnesDebugger::GetCallstackManager()
{
	return _callstackManager.get();
}

ITraceLogger* SnesDebugger::GetTraceLogger()
{
	return _traceLogger.get();
}

BreakpointManager* SnesDebugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

IAssembler* SnesDebugger::GetAssembler()
{
	return _assembler.get();
}

BaseEventManager* SnesDebugger::GetEventManager()
{
	return _eventManager.get();
}

CodeDataLogger* SnesDebugger::GetCodeDataLogger()
{
	if(_cpuType == CpuType::Sa1) {
		return _debugger->GetCodeDataLogger(CpuType::Snes);
	} else {
		return _codeDataLogger.get();
	}
}

PpuTools* SnesDebugger::GetPpuTools()
{
	return _ppuTools.get();
}

BaseState& SnesDebugger::GetState()
{
	return GetCpuState();
}

void SnesDebugger::GetPpuState(BaseState& state)
{
	(SnesPpuState&)state = _ppu->GetStateRef();
}

void SnesDebugger::SetPpuState(BaseState& srcState)
{
	SnesPpuState& dstState = _ppu->GetStateRef();
	dstState = (SnesPpuState&)srcState;
}