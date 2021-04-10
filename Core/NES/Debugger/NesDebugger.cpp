#include "stdafx.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
#include "Debugger/TraceLogger.h"
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
#include "Utilities/HexUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "EmuSettings.h"
#include "SettingTypes.h"
#include "MemoryMappings.h"
#include "Emulator.h"
#include "MemoryOperationType.h"

NesDebugger::NesDebugger(Debugger* debugger)
{
	_debugger = debugger;
	_emu = debugger->GetEmulator();
	NesConsole* console = (NesConsole*)debugger->GetConsole();
	
	_cpu = console->GetCpu();
	_ppu = console->GetPpu();
	_mapper = console->GetMapper();

	_traceLogger = debugger->GetTraceLogger().get();
	_disassembler = debugger->GetDisassembler().get();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter().get();
	_settings = debugger->GetEmulator()->GetSettings().get();

	_codeDataLogger.reset(new CodeDataLogger(_emu->GetMemory(SnesMemoryType::NesPrgRom).Size, CpuType::Nes));

	//_eventManager.reset(new EventManager(debugger, _cpu, console->GetPpu().get(), _memoryManager, console->GetDmaController().get()));
	_callstackManager.reset(new CallstackManager(debugger));
	_breakpointManager.reset(new BreakpointManager(debugger, CpuType::Nes, _eventManager.get()));
	_step.reset(new StepRequest());
	//_assembler.reset(new Assembler(_debugger->GetLabelManager()));

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

	NesCpuState state = _cpu->GetState();

	BreakSource breakSource = BreakSource::Unspecified;

	if(type == MemoryOperationType::ExecOpCode) {
		bool needDisassemble = _traceLogger->IsCpuLogged(CpuType::Nes) || _settings->CheckDebuggerFlag(DebuggerFlags::NesDebuggerEnabled);
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

		if(_traceLogger->IsCpuLogged(CpuType::Nes)) {
			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, state.PS, CpuType::Nes);
			_traceLogger->Log(CpuType::Nes, state, disInfo);
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
			if(value == 0x00 && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnBrk)) {
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
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _cpu->GetCycleCount());
	} else {
		if(addressInfo.Type == SnesMemoryType::NesPrgRom && addressInfo.Address >= 0) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Data);
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

	if(IsRegister(addr)) {
		//_eventManager->AddEvent(DebugEventType::Register, operation);
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

	if(IsRegister(addr)) {
		//_eventManager->AddEvent(DebugEventType::Register, operation);
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

void NesDebugger::ProcessPpuCycle(uint16_t& scanline, uint16_t& cycle)
{
	scanline = _ppu->GetCurrentScanline();
	cycle = _ppu->GetCurrentCycle();

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

bool NesDebugger::IsRegister(uint32_t addr)
{
	//TODO
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
