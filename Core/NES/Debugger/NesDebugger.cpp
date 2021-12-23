#include "stdafx.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/CodeDataLogger.h"
#include "Debugger/ScriptManager.h"
#include "Debugger/Debugger.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Debugger/CodeDataLogger.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"
#include "NES/NesPpu.h"
#include "NES/BaseMapper.h"
#include "NES/Debugger/NesDebugger.h"
#include "NES/Debugger/NesAssembler.h"
#include "NES/Debugger/NesEventManager.h"
#include "NES/Debugger/NesTraceLogger.h"
#include "NES/Debugger/NesPpuTools.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "Shared/EmuSettings.h"
#include "Shared/SettingTypes.h"
#include "Shared/Emulator.h"
#include "MemoryOperationType.h"

NesDebugger::NesDebugger(Debugger* debugger)
{
	_debugger = debugger;
	_emu = debugger->GetEmulator();
	NesConsole* console = (NesConsole*)debugger->GetConsole();
	
	_cpu = console->GetCpu();
	_ppu = console->GetPpu();
	_mapper = console->GetMapper();

	_traceLogger.reset(new NesTraceLogger(debugger, _ppu));
	_ppuTools.reset(new NesPpuTools(debugger, debugger->GetEmulator(), console));
	_disassembler = debugger->GetDisassembler().get();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter().get();
	_settings = debugger->GetEmulator()->GetSettings();

	_codeDataLogger.reset(new CodeDataLogger(SnesMemoryType::NesPrgRom, _emu->GetMemory(SnesMemoryType::NesPrgRom).Size, CpuType::Nes));

	_eventManager.reset(new NesEventManager(debugger, console));
	_callstackManager.reset(new CallstackManager(debugger));
	_breakpointManager.reset(new BreakpointManager(debugger, CpuType::Nes, _eventManager.get()));
	_step.reset(new StepRequest());
	_assembler.reset(new NesAssembler(_debugger->GetLabelManager()));

	if(_cpu->GetState().PC == 0) {
		//Enable breaking on uninit reads when debugger is opened at power on
		_enableBreakOnUninitRead = true;
	}
}

void NesDebugger::Reset()
{
	_enableBreakOnUninitRead = true;
	_callstackManager.reset(new CallstackManager(_debugger));
	_prevOpCode = 0xFF;
}

void NesDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _mapper->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation = { addr, value, type };
	NesCpuState& state = _cpu->GetState();
	BreakSource breakSource = BreakSource::Unspecified;

	if(IsRegister(operation)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	if(type == MemoryOperationType::ExecOpCode) {
		bool needDisassemble = _traceLogger->IsEnabled() || _settings->CheckDebuggerFlag(DebuggerFlags::NesDebuggerEnabled);
		if(addressInfo.Address >= 0) {
			if(addressInfo.Type == SnesMemoryType::NesPrgRom) {
				uint8_t flags = CdlFlags::Code;
				if(_prevOpCode == 0x20) {
					//JSR
					flags |= CdlFlags::SubEntryPoint;
				}
				_codeDataLogger->SetFlags(addressInfo.Address, flags);
			}
			if(needDisassemble) {
				_disassembler->BuildCache(addressInfo, 0, CpuType::Nes);
			}
		}

		if(_traceLogger->IsEnabled()) {
			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, state.PS, CpuType::Nes);
			_traceLogger->Log(state, disInfo, operation);
		}

		uint32_t pc = state.PC;
		if(_prevOpCode == 0x20) {
			//JSR
			uint8_t opSize = DisassemblyInfo::GetOpSize(_prevOpCode, state.PS, CpuType::Nes);
			uint32_t returnPc = (_prevProgramCounter & 0xFF0000) | (((_prevProgramCounter & 0xFFFF) + opSize) & 0xFFFF);
			AddressInfo srcAddress = _mapper->GetAbsoluteAddress(_prevProgramCounter);
			AddressInfo retAddress = _mapper->GetAbsoluteAddress(returnPc);
			_callstackManager->Push(srcAddress, _prevProgramCounter, addressInfo, pc, retAddress, returnPc, StackFrameFlags::None);
		} else if(_prevOpCode == 0x60 || _prevOpCode == 0x40) {
			//RTS, RTI
			_callstackManager->Pop(addressInfo, pc);
		}

		if(_step->BreakAddress == (int32_t)pc && (_prevOpCode == 0x60 || _prevOpCode == 0x40)) {
			//RTS/RTI found, if we're on the expected return address, break immediately (for step over/step out)
			_step->StepCount = 0;
		}

		_prevOpCode = value;
		_prevProgramCounter = pc;

		if(_step->StepCount > 0) {
			_step->StepCount--;
		}

		if(_settings->CheckDebuggerFlag(DebuggerFlags::NesDebuggerEnabled)) {
			if(value == 0x00 && _settings->CheckDebuggerFlag(DebuggerFlags::NesBreakOnBrk)) {
				//Break on BRK
				breakSource = BreakSource::BreakOnBrk;
				_step->StepCount = 0;
			}
		}
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _cpu->GetCycleCount());
	} else if(type == MemoryOperationType::ExecOperand) {
		if(addressInfo.Type == SnesMemoryType::NesPrgRom && addressInfo.Address >= 0) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Code);
		}

		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}

		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _cpu->GetCycleCount());
	} else {
		if(operation.Type == MemoryOperationType::DmaRead) {
			_eventManager->AddEvent(DebugEventType::DmcDmaRead, operation);
		}

		if(addressInfo.Type == SnesMemoryType::NesPrgRom && addressInfo.Address >= 0) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Data);
		}
		
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}

		if(_memoryAccessCounter->ProcessMemoryRead(addressInfo, _cpu->GetCycleCount())) {
			//Memory access was a read on an uninitialized memory address
			if(_enableBreakOnUninitRead) {
				if(_memoryAccessCounter->GetReadCount(addressInfo) == 1) {
					//Only warn the first time
					_debugger->Log("[CPU] Uninitialized memory read: $" + HexUtilities::ToHex24(addr));
				}
				if(_settings->CheckDebuggerFlag(DebuggerFlags::NesDebuggerEnabled) && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnUninitRead)) {
					breakSource = BreakSource::BreakOnUninitMemoryRead;
					_step->StepCount = 0;
				}
			}
		}
	}

	_debugger->ProcessBreakConditions(_step->StepCount == 0, _breakpointManager.get(), operation, addressInfo, breakSource);
}

void NesDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _mapper->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation = { addr, value, type };
	if(addressInfo.Address >= 0 && (addressInfo.Type == SnesMemoryType::WorkRam || addressInfo.Type == SnesMemoryType::SaveRam)) {
		_disassembler->InvalidateCache(addressInfo, CpuType::Nes);
	}

	if(IsRegister(operation)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation);
	}

	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _cpu->GetCycleCount());

	_debugger->ProcessBreakConditions(false, _breakpointManager.get(), operation, addressInfo);
}

void NesDebugger::Run()
{
	_step.reset(new StepRequest());
}

void NesDebugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step;
	switch(type) {
		case StepType::Step: step.StepCount = stepCount; break;
		case StepType::StepOut: step.BreakAddress = _callstackManager->GetReturnAddress(); break;
		case StepType::StepOver:
			if(_prevOpCode == 0x20 || _prevOpCode == 0x00) {
				//JSR, BRK
				step.BreakAddress = (_prevProgramCounter & 0xFF0000) | (((_prevProgramCounter & 0xFFFF) + DisassemblyInfo::GetOpSize(_prevOpCode, 0, CpuType::Nes)) & 0xFFFF);
			} else {
				//For any other instruction, step over is the same as step into
				step.StepCount = 1;
			}
			break;

		case StepType::PpuStep: step.PpuStepCount = stepCount; break;
		case StepType::PpuScanline: step.PpuStepCount = 341; break;
		case StepType::PpuFrame: step.PpuStepCount = 341 * _ppu->GetScanlineCount(); break;
		case StepType::SpecificScanline: step.BreakScanline = stepCount; break;
	}
	_step.reset(new StepRequest(step));
}

void NesDebugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	AddressInfo src = _mapper->GetAbsoluteAddress(_prevProgramCounter);
	AddressInfo ret = _mapper->GetAbsoluteAddress(originalPc);
	AddressInfo dest = _mapper->GetAbsoluteAddress(currentPc);
	_callstackManager->Push(src, _prevProgramCounter, dest, currentPc, ret, originalPc, forNmi ? StackFrameFlags::Nmi : StackFrameFlags::Irq);
	//_eventManager->AddEvent(forNmi ? DebugEventType::Nmi : DebugEventType::Irq);
}

void NesDebugger::ProcessPpuCycle()
{
	int16_t scanline = _ppu->GetCurrentScanline();
	uint16_t cycle = _ppu->GetCurrentCycle();
	_ppuTools->UpdateViewers(scanline, cycle);

	if(cycle == 0 && scanline == _step->BreakScanline) {
		_debugger->SleepUntilResume(BreakSource::PpuStep);
	} else if(_step->PpuStepCount > 0) {
		_step->PpuStepCount--;
		if(_step->PpuStepCount == 0) {
			_debugger->SleepUntilResume(BreakSource::PpuStep);
		}
	}
}

bool NesDebugger::IsRegister(MemoryOperationInfo& op)
{
	if(op.Address >= 0x2000 && op.Address <= 0x3FFF) {
		return true;
	} else if(op.Address >= 0x4000 && op.Address <= 0x4015 || (op.Address == 0x4017 && op.Type == MemoryOperationType::Write)) {
		return true;
	} else if(op.Address == 0x4016 || (op.Address == 0x4017 && op.Type == MemoryOperationType::Read)) {
		return true;
	} else if(op.Address >= 0x4018 && ((op.Type == MemoryOperationType::Write && _mapper->IsWriteRegister(op.Address)) || (op.Type == MemoryOperationType::Read && _mapper->IsReadRegister(op.Address)))) {
		return true;
	}
	return false;
}

shared_ptr<CallstackManager> NesDebugger::GetCallstackManager()
{
	return _callstackManager;
}

BreakpointManager* NesDebugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

shared_ptr<IAssembler> NesDebugger::GetAssembler()
{
	return _assembler;
}

shared_ptr<IEventManager> NesDebugger::GetEventManager()
{
	return _eventManager;
}

shared_ptr<CodeDataLogger> NesDebugger::GetCodeDataLogger()
{
	return _codeDataLogger;
}

BaseState& NesDebugger::GetState()
{
	return _cpu->GetState();
}

void NesDebugger::GetPpuState(BaseState& state)
{
	_ppu->GetState((NesPpuState&)state);
}

ITraceLogger* NesDebugger::GetTraceLogger()
{
	return _traceLogger.get();
}

PpuTools* NesDebugger::GetPpuTools()
{
	return _ppuTools.get();
}
