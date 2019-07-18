#include "stdafx.h"
#include "Debugger.h"
#include "DebugTypes.h"
#include "Console.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Spc.h"
#include "NecDsp.h"
#include "BaseCartridge.h"
#include "MemoryManager.h"
#include "EmuSettings.h"
#include "SoundMixer.h"
#include "NotificationManager.h"
#include "CpuTypes.h"
#include "DisassemblyInfo.h"
#include "TraceLogger.h"
#include "MemoryDumper.h"
#include "MemoryAccessCounter.h"
#include "CodeDataLogger.h"
#include "Disassembler.h"
#include "BreakpointManager.h"
#include "PpuTools.h"
#include "EventManager.h"
#include "EventType.h"
#include "DebugBreakHelper.h"
#include "LabelManager.h"
#include "ScriptManager.h"
#include "CallstackManager.h"
#include "ExpressionEvaluator.h"
#include "../Utilities/HexUtilities.h"
#include "../Utilities/FolderUtilities.h"

Debugger::Debugger(shared_ptr<Console> console)
{
	_console = console;
	_cpu = console->GetCpu();
	_ppu = console->GetPpu();
	_spc = console->GetSpc();
	_cart = console->GetCartridge();
	_settings = console->GetSettings();
	_memoryManager = console->GetMemoryManager();

	_labelManager.reset(new LabelManager(this));
	_watchExpEval[(int)CpuType::Cpu].reset(new ExpressionEvaluator(this, CpuType::Cpu));
	_watchExpEval[(int)CpuType::Spc].reset(new ExpressionEvaluator(this, CpuType::Spc));

	_codeDataLogger.reset(new CodeDataLogger(_cart->DebugGetPrgRomSize(), _memoryManager.get()));
	_memoryDumper.reset(new MemoryDumper(_ppu, console->GetSpc(), _memoryManager, _cart));
	_disassembler.reset(new Disassembler(console, _codeDataLogger, this));
	_traceLogger.reset(new TraceLogger(this, _console));
	_memoryAccessCounter.reset(new MemoryAccessCounter(this, _spc.get(), _memoryManager.get()));
	_breakpointManager.reset(new BreakpointManager(this));
	_ppuTools.reset(new PpuTools(_console.get(), _ppu.get()));
	_eventManager.reset(new EventManager(this, _cpu.get(), _ppu.get(), _console->GetDmaController().get()));
	_scriptManager.reset(new ScriptManager(this));
	_callstackManager.reset(new CallstackManager(this));
	_spcCallstackManager.reset(new CallstackManager(this));

	_step.reset(new StepRequest());

	_executionStopped = false;
	_breakRequestCount = 0;
	_suspendRequestCount = 0;

	if(_cpu->GetState().PC == 0) {
		//Enable breaking on uninit reads when debugger is opened at power on
		_enableBreakOnUninitRead = true;
	}

	string cdlFile = FolderUtilities::CombinePath(FolderUtilities::GetDebuggerFolder(), FolderUtilities::GetFilename(_cart->GetRomInfo().RomFile.GetFileName(), false) + ".cdl");
	_codeDataLogger->LoadCdlFile(cdlFile);

	RefreshCodeCache();
}

Debugger::~Debugger()
{
	Release();
}

void Debugger::Release()
{
	string cdlFile = FolderUtilities::CombinePath(FolderUtilities::GetDebuggerFolder(), FolderUtilities::GetFilename(_cart->GetRomInfo().RomFile.GetFileName(), false) + ".cdl");
	_codeDataLogger->SaveCdlFile(cdlFile);

	while(_executionStopped) {
		Run();
	}
}

void Debugger::Reset()
{
	_enableBreakOnUninitRead = true;
	_prevOpCode = 0xFF;
	_spcPrevOpCode = 0xFF;
}

void Debugger::ProcessCpuRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _memoryManager->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation = { addr, value, type };
	CpuState state = _cpu->GetState();
	BreakSource breakSource = BreakSource::Unspecified;

	if(type == MemoryOperationType::ExecOpCode) {
		if(addressInfo.Address >= 0) {
			if(addressInfo.Type == SnesMemoryType::PrgRom) {
				uint8_t flags = CdlFlags::Code | (state.PS & (CdlFlags::IndexMode8 | CdlFlags::MemoryMode8));
				if(_prevOpCode == 0x20 || _prevOpCode == 0x22 || _prevOpCode == 0xFC) {
					flags |= CdlFlags::SubEntryPoint;
				}
				_codeDataLogger->SetFlags(addressInfo.Address, flags);
			}
			_disassembler->BuildCache(addressInfo, state.PS, CpuType::Cpu);
		}

		DebugState debugState;
		GetState(debugState, true);

		DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo);
		_traceLogger->Log(debugState, disInfo);

		uint32_t pc = (state.K << 16) | state.PC;
		if(_prevOpCode == 0x20 || _prevOpCode == 0x22 || _prevOpCode == 0xFC) {
			//JSR, JSL
			uint8_t opSize = DisassemblyInfo::GetOpSize(_prevOpCode, state.PS, CpuType::Cpu);
			uint32_t returnPc = (_prevProgramCounter & 0xFF0000) | (((_prevProgramCounter & 0xFFFF) + opSize) & 0xFFFF);
			_callstackManager->Push(_prevProgramCounter, pc, returnPc, StackFrameFlags::None);
		} else if(_prevOpCode == 0x60 || _prevOpCode == 0x6B || _prevOpCode == 0x40) {
			//RTS, RTL, RTI
			_callstackManager->Pop(pc);
		}

		ProcessStepConditions(_prevOpCode, pc);

		_prevOpCode = value;
		_prevProgramCounter = pc;

		if(_step->CpuStepCount > 0) {
			_step->CpuStepCount--;
		}
		
		if(_settings->CheckDebuggerFlag(DebuggerFlags::CpuDebuggerEnabled)) {
			if(value == 0x00 || value == 0x02 || value == 0x42 || value == 0xDB) {
				//Break on BRK/STP/WDM/COP
				if(value == 0x00 && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnBrk)) {
					breakSource = BreakSource::BreakOnBrk;
					_step->CpuStepCount = 0;
				} else if(value == 0x02 && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnCop)) {
					breakSource = BreakSource::BreakOnCop;
					_step->CpuStepCount = 0;
				} else if(value == 0x42 && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnWdm)) {
					breakSource = BreakSource::BreakOnWdm;
					_step->CpuStepCount = 0;
				} else if(value == 0xDB && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnStp)) {
					breakSource = BreakSource::BreakOnStp;
					_step->CpuStepCount = 0;
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
			_step->CpuStepCount = 0;
		}
	}

	if(_memoryManager->IsRegister(addr)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	ProcessBreakConditions(operation, addressInfo, breakSource);

	_scriptManager->ProcessMemoryOperation(addr, value, type);
}

void Debugger::ProcessCpuWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _memoryManager->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation = { addr, value, type };
	if(addressInfo.Address >= 0 && (addressInfo.Type == SnesMemoryType::WorkRam || addressInfo.Type == SnesMemoryType::SaveRam)) {
		_disassembler->InvalidateCache(addressInfo, CpuType::Cpu);
	}

	if(_memoryManager->IsRegister(addr)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, type, _memoryManager->GetMasterClock());

	ProcessBreakConditions(operation, addressInfo);

	_scriptManager->ProcessMemoryOperation(addr, value, type);
}

void Debugger::ProcessWorkRamRead(uint32_t addr, uint8_t value)
{
	AddressInfo addressInfo { (int32_t)addr, SnesMemoryType::WorkRam };
	//TODO Make this more flexible/accurate
	MemoryOperationInfo operation { 0x7E0000 | addr, value, MemoryOperationType::Read };
	ProcessBreakConditions(operation, addressInfo);
}

void Debugger::ProcessWorkRamWrite(uint32_t addr, uint8_t value)
{
	AddressInfo addressInfo { (int32_t)addr, SnesMemoryType::WorkRam };
	//TODO Make this more flexible/accurate
	MemoryOperationInfo operation { 0x7E0000 | addr, value, MemoryOperationType::Write };
	ProcessBreakConditions(operation, addressInfo);
}

void Debugger::ProcessSpcRead(uint16_t addr, uint8_t value, MemoryOperationType type)
{
	if(type == MemoryOperationType::DummyRead) {
		//Ignore all dummy reads for now
		return;
	}

	AddressInfo addressInfo = _spc->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation { addr, value, type };

	if(type == MemoryOperationType::ExecOpCode) {
		_disassembler->BuildCache(addressInfo, 0, CpuType::Spc);

		DebugState debugState;
		GetState(debugState, true);

		DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo);
		_traceLogger->Log(debugState, disInfo);

		if(_spcPrevOpCode == 0x3F || _spcPrevOpCode == 0x0F) {
			//JSR, BRK
			uint8_t opSize = DisassemblyInfo::GetOpSize(_spcPrevOpCode, 0, CpuType::Spc);
			uint16_t returnPc = _spcPrevProgramCounter + opSize;
			_spcCallstackManager->Push(_spcPrevProgramCounter, debugState.Spc.PC, returnPc, StackFrameFlags::None);
		} else if(_spcPrevOpCode == 0x6F || _spcPrevOpCode == 0x7F) {
			//RTS, RTI
			_spcCallstackManager->Pop(debugState.Spc.PC);
		}

		if(_step->SpcBreakAddress == (int32_t)debugState.Spc.PC && (_spcPrevOpCode == 0x6F || _spcPrevOpCode == 0x7F)) {
			//RTS/RTI found, if we're on the expected return address, break immediately (for step over/step out)
			_step->CpuStepCount = 0;
		}

		_spcPrevOpCode = value;
		_spcPrevProgramCounter = debugState.Spc.PC;

		if(_step->SpcStepCount > 0) {
			_step->SpcStepCount--;
			if(_step->SpcStepCount == 0) {
				_step->SpcStepCount = -1;
				_step->CpuStepCount = 0;
			}
		}
	}

	ProcessBreakConditions(operation, addressInfo);

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, type, _memoryManager->GetMasterClock());
}

void Debugger::ProcessSpcWrite(uint16_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo { addr, SnesMemoryType::SpcRam }; //Writes never affect the SPC ROM
	MemoryOperationInfo operation { addr, value, type };
	ProcessBreakConditions(operation, addressInfo);

	_disassembler->InvalidateCache(addressInfo, CpuType::Spc);

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, type, _memoryManager->GetMasterClock());
}

void Debugger::ProcessPpuRead(uint16_t addr, uint8_t value, SnesMemoryType memoryType)
{
	AddressInfo addressInfo { addr, memoryType };
	MemoryOperationInfo operation { addr, value, MemoryOperationType::Read };
	ProcessBreakConditions(operation, addressInfo);

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, MemoryOperationType::Read, _memoryManager->GetMasterClock());
}

void Debugger::ProcessPpuWrite(uint16_t addr, uint8_t value, SnesMemoryType memoryType)
{
	AddressInfo addressInfo { addr, memoryType };
	MemoryOperationInfo operation { addr, value, MemoryOperationType::Write };
	ProcessBreakConditions(operation, addressInfo);

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, MemoryOperationType::Write, _memoryManager->GetMasterClock());
}

void Debugger::ProcessPpuCycle()
{
	uint16_t scanline = _ppu->GetScanline();
	uint16_t cycle = _ppu->GetCycle();
	_ppuTools->UpdateViewers(scanline, cycle);

	if(_step->PpuStepCount > 0) {
		_step->PpuStepCount--;
		if(_step->PpuStepCount == 0) {
			_step->CpuStepCount = 0;
			SleepUntilResume(BreakSource::PpuStep);
		}
	}

	if(cycle == 0 && scanline == _step->BreakScanline) {
		_step->CpuStepCount = 0;
		_step->BreakScanline = -1;
		SleepUntilResume(BreakSource::PpuStep);
	}
}

void Debugger::ProcessNecDspExec(uint32_t addr, uint32_t value)
{
	if(_traceLogger->IsCpuLogged(CpuType::NecDsp)) {
		AddressInfo addressInfo { (int32_t)addr * 4, SnesMemoryType::DspProgramRom };

		_disassembler->BuildCache(addressInfo, 0, CpuType::NecDsp);

		DebugState debugState;
		GetState(debugState, true);

		DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo);
		_traceLogger->Log(debugState, disInfo);
	}
}

void Debugger::SleepUntilResume(BreakSource source, MemoryOperationInfo *operation, int breakpointId)
{
	if(_suspendRequestCount) {
		return;
	}

	_console->GetSoundMixer()->StopAudio();
	_disassembler->Disassemble(CpuType::Cpu);
	_disassembler->Disassemble(CpuType::Spc);

	_executionStopped = true;
	
	if(_step->CpuStepCount == 0) {
		//Only trigger code break event if the pause was caused by user action
		BreakEvent evt = {};
		evt.BreakpointId = breakpointId;
		evt.Source = source;
		if(operation) {
			evt.Operation = *operation;
		}
		_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::CodeBreak, &evt);
	}

	while((_step->CpuStepCount == 0 && !_suspendRequestCount) || _breakRequestCount) {
		std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(10));
	}

	_executionStopped = false;
}

void Debugger::ProcessStepConditions(uint8_t opCode, uint32_t currentPc)
{
	if(_step->CpuBreakAddress == (int32_t)currentPc && (opCode == 0x60 || opCode == 0x40 || opCode == 0x6B || opCode == 0x44 || opCode == 0x54)) {
		//RTS/RTL/RTI found, if we're on the expected return address, break immediately (for step over/step out)
		_step->CpuStepCount = 0;
	}
}

void Debugger::ProcessBreakConditions(MemoryOperationInfo &operation, AddressInfo &addressInfo, BreakSource source)
{
	if(_step->CpuStepCount == 0 || _breakRequestCount) {
		SleepUntilResume(source);
	} else {
		int breakpointId = _breakpointManager->CheckBreakpoint(operation, addressInfo);
		if(breakpointId >= 0) {
			_step->CpuStepCount = 0;
			SleepUntilResume(BreakSource::Breakpoint, &operation, breakpointId);
		}
	}
}

void Debugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	_callstackManager->Push(_prevProgramCounter, currentPc, originalPc, forNmi ? StackFrameFlags::Nmi : StackFrameFlags::Irq);	
	_eventManager->AddEvent(forNmi ? DebugEventType::Nmi : DebugEventType::Irq);
}

void Debugger::ProcessEvent(EventType type)
{
	_scriptManager->ProcessEvent(type);

	switch(type) {
		default: break;

		case EventType::StartFrame:
			_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::EventViewerRefresh);
			_eventManager->ClearFrameEvents();
			break;

		case EventType::Reset:
			Reset();
			break;
	}
}

int32_t Debugger::EvaluateExpression(string expression, CpuType cpuType, EvalResultType &resultType, bool useCache)
{
	MemoryOperationInfo operationInfo { 0, 0, MemoryOperationType::Read };
	DebugState state;
	GetState(state, false);
	if(useCache) {
		return _watchExpEval[(int)cpuType]->Evaluate(expression, state, resultType, operationInfo);
	} else {
		ExpressionEvaluator expEval(this, cpuType);
		return expEval.Evaluate(expression, state, resultType, operationInfo);
	}
}

void Debugger::Run()
{
	_step.reset(new StepRequest());
}

void Debugger::Step(int32_t stepCount, StepType type)
{
	if((type == StepType::CpuStepOver || type == StepType::CpuStepOut || type == StepType::CpuStep) && _cpu->GetState().StopState == CpuStopState::Stopped) {
		//If STP was called, the CPU isn't running anymore - use the PPU to break execution instead (useful for test roms that end with STP)
		type = StepType::PpuStep;
		stepCount = 1;
	}

	StepRequest step;

	switch(type) {
		case StepType::CpuStep: step.CpuStepCount = stepCount; break;
		case StepType::CpuStepOut: step.CpuBreakAddress = _callstackManager->GetReturnAddress(); break;
		case StepType::CpuStepOver:
			if(_prevOpCode == 0x20 || _prevOpCode == 0x22 || _prevOpCode == 0xFC || _prevOpCode == 0x00 || _prevOpCode == 0x02 || _prevOpCode == 0x44 || _prevOpCode == 0x54) {
				//JSR, JSL, BRK, COP, MVP, MVN
				step.CpuBreakAddress = (_prevProgramCounter & 0xFF0000) | (((_prevProgramCounter & 0xFFFF) + DisassemblyInfo::GetOpSize(_prevOpCode, 0, CpuType::Cpu)) & 0xFFFF);
			} else {
				//For any other instruction, step over is the same as step into
				step.CpuStepCount = 1;
			}
			break;

		case StepType::SpcStep: step.SpcStepCount = stepCount; break;
		case StepType::SpcStepOut: step.SpcBreakAddress = _spcCallstackManager->GetReturnAddress(); break;
		case StepType::SpcStepOver:
			if(_spcPrevOpCode == 0x3F || _spcPrevOpCode == 0x0F) {
				//JSR, BRK
				step.SpcBreakAddress = _spcPrevProgramCounter + DisassemblyInfo::GetOpSize(_spcPrevOpCode, 0, CpuType::Spc);
			} else {
				//For any other instruction, step over is the same as step into
				step.SpcStepCount = 1;
			}
			break;

		case StepType::PpuStep: step.PpuStepCount = stepCount; break;
		case StepType::SpecificScanline: step.BreakScanline = stepCount; break;
	}

	_step.reset(new StepRequest(step));
}

bool Debugger::IsExecutionStopped()
{
	return _executionStopped;
}

void Debugger::BreakRequest(bool release)
{
	if(release) {
		_breakRequestCount--;
	} else {
		_breakRequestCount++;
	}
}

void Debugger::SuspendDebugger(bool release)
{
	if(release) {
		_suspendRequestCount--;
	} else {
		_suspendRequestCount++;
	}
}

void Debugger::GetState(DebugState &state, bool partialPpuState)
{
	state.MasterClock = _memoryManager->GetMasterClock();
	state.Cpu = _cpu->GetState();
	_ppu->GetState(state.Ppu, partialPpuState);
	state.Spc = _spc->GetState();
	if(_cart->GetDsp()) {
		state.Dsp = _cart->GetDsp()->GetState();
	}
}

AddressInfo Debugger::GetAbsoluteAddress(AddressInfo relAddress)
{
	if(relAddress.Type == SnesMemoryType::CpuMemory) {
		if(_memoryManager->IsRegister(relAddress.Address)) {
			return { relAddress.Address & 0xFFFF, SnesMemoryType::Register };
		} else {
			return _memoryManager->GetAbsoluteAddress(relAddress.Address);
		}
	} else if(relAddress.Type == SnesMemoryType::SpcMemory) {
		return _spc->GetAbsoluteAddress(relAddress.Address);
	}

	throw std::runtime_error("Unsupported address type");
}

AddressInfo Debugger::GetRelativeAddress(AddressInfo absAddress)
{
	switch(absAddress.Type) {
		case SnesMemoryType::PrgRom:
		case SnesMemoryType::WorkRam:
		case SnesMemoryType::SaveRam:
			return { _memoryManager->GetRelativeAddress(absAddress), SnesMemoryType::CpuMemory };

		case SnesMemoryType::SpcRam:
		case SnesMemoryType::SpcRom:
			return { _spc->GetRelativeAddress(absAddress), SnesMemoryType::SpcMemory };

		case SnesMemoryType::Register:
			return { -1, SnesMemoryType::Register };

		default: 
			throw std::runtime_error("Unsupported address type");
	}
}

void Debugger::SetCdlData(uint8_t *cdlData, uint32_t length)
{
	DebugBreakHelper helper(this);
	_codeDataLogger->SetCdlData(cdlData, length);
	RefreshCodeCache();
}

void Debugger::RefreshCodeCache()
{
	_disassembler->ResetPrgCache();
	uint32_t prgRomSize = _cart->DebugGetPrgRomSize();
	AddressInfo addrInfo;
	addrInfo.Type = SnesMemoryType::PrgRom;

	for(uint32_t i = 0; i < prgRomSize; i++) {
		if(_codeDataLogger->IsCode(i)) {
			addrInfo.Address = (int32_t)i;
			i += _disassembler->BuildCache(addrInfo, _codeDataLogger->GetCpuFlags(i), CpuType::Cpu) - 1;
		}
	}
}

shared_ptr<TraceLogger> Debugger::GetTraceLogger()
{
	return _traceLogger;
}

shared_ptr<MemoryDumper> Debugger::GetMemoryDumper()
{
	return _memoryDumper;
}

shared_ptr<MemoryAccessCounter> Debugger::GetMemoryAccessCounter()
{
	return _memoryAccessCounter;
}

shared_ptr<CodeDataLogger> Debugger::GetCodeDataLogger()
{
	return _codeDataLogger;
}

shared_ptr<Disassembler> Debugger::GetDisassembler()
{
	return _disassembler;
}

shared_ptr<BreakpointManager> Debugger::GetBreakpointManager()
{
	return _breakpointManager;
}

shared_ptr<PpuTools> Debugger::GetPpuTools()
{
	return _ppuTools;
}

shared_ptr<EventManager> Debugger::GetEventManager()
{
	return _eventManager;
}

shared_ptr<LabelManager> Debugger::GetLabelManager()
{
	return _labelManager;
}

shared_ptr<ScriptManager> Debugger::GetScriptManager()
{
	return _scriptManager;
}

shared_ptr<CallstackManager> Debugger::GetCallstackManager(CpuType cpuType)
{
	return cpuType == CpuType::Cpu ? _callstackManager : _spcCallstackManager;
}

shared_ptr<Console> Debugger::GetConsole()
{
	return _console;
}