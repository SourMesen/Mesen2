#include "stdafx.h"
#include "Debugger.h"
#include "DebugTypes.h"
#include "Console.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Spc.h"
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
	_settings = console->GetSettings();
	_memoryManager = console->GetMemoryManager();

	_labelManager.reset(new LabelManager(this));
	_watchExpEval[(int)CpuType::Cpu].reset(new ExpressionEvaluator(this, CpuType::Cpu));
	_watchExpEval[(int)CpuType::Spc].reset(new ExpressionEvaluator(this, CpuType::Spc));

	_codeDataLogger.reset(new CodeDataLogger(console->GetCartridge()->DebugGetPrgRomSize(), _memoryManager.get()));
	_disassembler.reset(new Disassembler(console, _codeDataLogger, this));
	_traceLogger.reset(new TraceLogger(this, _console));
	_memoryDumper.reset(new MemoryDumper(_ppu, console->GetSpc(), _memoryManager, console->GetCartridge()));
	_memoryAccessCounter.reset(new MemoryAccessCounter(this, _spc.get(), _memoryManager.get()));
	_breakpointManager.reset(new BreakpointManager(this));
	_ppuTools.reset(new PpuTools(_console.get(), _ppu.get()));
	_eventManager.reset(new EventManager(this, _cpu.get(), _ppu.get()));
	_callstackManager.reset(new CallstackManager(this));
	_spcCallstackManager.reset(new CallstackManager(this));
	
	_cpuStepCount = -1;
	_spcStepCount = -1;
	_ppuStepCount = -1;
	_cpuBreakAddress = -1;
	_spcBreakAddress = -1;
	_breakScanline = -1;

	_executionStopped = false;
	_breakRequestCount = 0;
	_suspendRequestCount = 0;

	string cdlFile = FolderUtilities::CombinePath(FolderUtilities::GetDebuggerFolder(), FolderUtilities::GetFilename(_console->GetCartridge()->GetRomInfo().RomFile.GetFileName(), false) + ".cdl");
	_codeDataLogger->LoadCdlFile(cdlFile);

	RefreshCodeCache();
}

Debugger::~Debugger()
{
	Release();
}

void Debugger::Release()
{
	string cdlFile = FolderUtilities::CombinePath(FolderUtilities::GetDebuggerFolder(), FolderUtilities::GetFilename(_console->GetCartridge()->GetRomInfo().RomFile.GetFileName(), false) + ".cdl");
	_codeDataLogger->SaveCdlFile(cdlFile);

	while(_executionStopped) {
		Run();
	}
}

void Debugger::ProcessCpuRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _memoryManager->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation = { addr, value, type };
	CpuState state = _cpu->GetState();

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
		GetState(debugState);

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

		if(_cpuStepCount > 0) {
			_cpuStepCount--;
		}
		
		if(_settings->CheckDebuggerFlag(DebuggerFlags::CpuDebuggerEnabled)) {
			if(value == 0x00 || value == 0x02 || value == 0x42 || value == 0xDB) {
				//Break on BRK/STP/WDM/COP
				if(value == 0x00 && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnBrk)) {
					_cpuStepCount = 0;
				} else if(value == 0x02 && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnCop)) {
					_cpuStepCount = 0;
				} else if(value == 0x42 && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnWdm)) {
					_cpuStepCount = 0;
				} else if(value == 0xDB && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnStp)) {
					_cpuStepCount = 0;
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

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, type, _memoryManager->GetMasterClock());

	if(_memoryManager->IsRegister(addr)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	ProcessBreakConditions(operation, addressInfo);
}

void Debugger::ProcessCpuWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _memoryManager->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation = { addr, value, type };
	if(addressInfo.Address >= 0 && (addressInfo.Type == SnesMemoryType::WorkRam || addressInfo.Type == SnesMemoryType::SaveRam)) {
		_disassembler->InvalidateCache(addressInfo);
	}

	if(_memoryManager->IsRegister(addr)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, type, _memoryManager->GetMasterClock());

	ProcessBreakConditions(operation, addressInfo);
}

void Debugger::ProcessWorkRamRead(uint32_t addr, uint8_t value)
{
	AddressInfo addressInfo { (int32_t)addr, SnesMemoryType::WorkRam };
	//TODO Make this more flexible/accurate
	MemoryOperationInfo operation(0x7E0000 | addr, value, MemoryOperationType::Read);
	ProcessBreakConditions(operation, addressInfo);
}

void Debugger::ProcessWorkRamWrite(uint32_t addr, uint8_t value)
{
	AddressInfo addressInfo { (int32_t)addr, SnesMemoryType::WorkRam };
	//TODO Make this more flexible/accurate
	MemoryOperationInfo operation(0x7E0000 | addr, value, MemoryOperationType::Write);
	ProcessBreakConditions(operation, addressInfo);
}

void Debugger::ProcessSpcRead(uint16_t addr, uint8_t value, MemoryOperationType type)
{
	if(type == MemoryOperationType::DummyRead) {
		//Ignore all dummy reads for now
		return;
	}

	AddressInfo addressInfo = _spc->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type);

	if(type == MemoryOperationType::ExecOpCode) {
		_disassembler->BuildCache(addressInfo, 0, CpuType::Spc);

		DebugState debugState;
		GetState(debugState);

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

		if(_spcBreakAddress == (int32_t)debugState.Spc.PC && (_spcPrevOpCode == 0x6F || _spcPrevOpCode == 0x7F)) {
			//RTS/RTI found, if we're on the expected return address, break immediately (for step over/step out)
			_cpuStepCount = 0;
		}

		_spcPrevOpCode = value;
		_spcPrevProgramCounter = debugState.Spc.PC;

		if(_spcStepCount > 0) {
			_spcStepCount--;
			if(_spcStepCount == 0) {
				_spcStepCount = -1;
				_cpuStepCount = 0;
			}
		}
	}

	ProcessBreakConditions(operation, addressInfo);

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, type, _memoryManager->GetMasterClock());
}

void Debugger::ProcessSpcWrite(uint16_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo { addr, SnesMemoryType::SpcRam }; //Writes never affect the SPC ROM
	MemoryOperationInfo operation(addr, value, type);
	ProcessBreakConditions(operation, addressInfo);

	_disassembler->InvalidateCache(addressInfo);

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, type, _memoryManager->GetMasterClock());
}

void Debugger::ProcessPpuRead(uint16_t addr, uint8_t value, SnesMemoryType memoryType)
{
	AddressInfo addressInfo { addr, memoryType };
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Read);
	ProcessBreakConditions(operation, addressInfo);

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, MemoryOperationType::Read, _memoryManager->GetMasterClock());
}

void Debugger::ProcessPpuWrite(uint16_t addr, uint8_t value, SnesMemoryType memoryType)
{
	AddressInfo addressInfo { addr, memoryType };
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Write);
	ProcessBreakConditions(operation, addressInfo);

	_memoryAccessCounter->ProcessMemoryAccess(addressInfo, MemoryOperationType::Write, _memoryManager->GetMasterClock());
}

void Debugger::ProcessPpuCycle()
{
	uint16_t scanline = _ppu->GetScanline();
	uint16_t cycle = _ppu->GetCycle();
	_ppuTools->UpdateViewers(scanline, cycle);

	if(_ppuStepCount > 0) {
		_ppuStepCount--;
		if(_ppuStepCount == 0) {
			_cpuStepCount = 0;
			SleepUntilResume();
		}
	}

	if(cycle == 0 && scanline == _breakScanline) {
		_cpuStepCount = 0;
		_breakScanline = -1;
		SleepUntilResume();
	}
}

void Debugger::SleepUntilResume()
{
	if(_suspendRequestCount) {
		return;
	}

	_console->GetSoundMixer()->StopAudio();
	_disassembler->Disassemble(CpuType::Cpu);
	_disassembler->Disassemble(CpuType::Spc);

	_executionStopped = true;
	
	if(_cpuStepCount == 0) {
		//Only trigger code break event if the pause was caused by user action
		_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::CodeBreak);
	}

	while((_cpuStepCount == 0 && !_suspendRequestCount) || _breakRequestCount) {
		std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(10));
	}

	_executionStopped = false;
}

void Debugger::ProcessStepConditions(uint8_t opCode, uint32_t currentPc)
{
	if(_cpuBreakAddress == (int32_t)currentPc && (opCode == 0x60 || opCode == 0x40 || opCode == 0x6B || opCode == 0x44 || opCode == 0x54)) {
		//RTS/RTL/RTI found, if we're on the expected return address, break immediately (for step over/step out)
		_cpuStepCount = 0;
	}
}

void Debugger::ProcessBreakConditions(MemoryOperationInfo &operation, AddressInfo &addressInfo)
{
	if(_breakpointManager->CheckBreakpoint(operation, addressInfo)) {
		_cpuStepCount = 0;
	}

	if(_cpuStepCount == 0 || _breakRequestCount) {
		SleepUntilResume();
	}
}

void Debugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	_callstackManager->Push(_prevProgramCounter, currentPc, originalPc, forNmi ? StackFrameFlags::Nmi : StackFrameFlags::Irq);	
	_eventManager->AddEvent(forNmi ? DebugEventType::Nmi : DebugEventType::Irq);
}

void Debugger::ProcessEvent(EventType type)
{
	switch(type) {
		default: break;

		case EventType::StartFrame:
			_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::EventViewerRefresh);
			_eventManager->ClearFrameEvents();
			break;
	}
}

int32_t Debugger::EvaluateExpression(string expression, CpuType cpuType, EvalResultType &resultType, bool useCache)
{
	MemoryOperationInfo operationInfo { 0, 0, MemoryOperationType::Read };
	DebugState state;
	GetState(state);
	if(useCache) {
		return _watchExpEval[(int)cpuType]->Evaluate(expression, state, resultType, operationInfo);
	} else {
		ExpressionEvaluator expEval(this, cpuType);
		return expEval.Evaluate(expression, state, resultType, operationInfo);
	}
}

void Debugger::Run()
{
	_cpuStepCount = -1;
	_spcStepCount = -1;
	_ppuStepCount = -1;
	_cpuBreakAddress = -1;
	_spcBreakAddress = -1;
	_breakScanline = -1;
}

void Debugger::Step(int32_t stepCount, StepType type)
{
	switch(type) {
		case StepType::CpuStep:
			_cpuStepCount = stepCount;
			_cpuBreakAddress = -1;
			_spcBreakAddress = -1;
			_spcStepCount = -1;
			_ppuStepCount = -1;
			_breakScanline = -1;
			break;

		case StepType::CpuStepOut:
			_cpuBreakAddress = _callstackManager->GetReturnAddress();
			_cpuStepCount = -1;
			_spcStepCount = -1;
			_ppuStepCount = -1;
			_spcBreakAddress = -1;
			_breakScanline = -1;
			break;

		case StepType::CpuStepOver:
			if(_prevOpCode == 0x20 || _prevOpCode == 0x22 || _prevOpCode == 0xFC || _prevOpCode == 0x00 || _prevOpCode == 0x02 || _prevOpCode == 0x44 || _prevOpCode == 0x54) {
				//JSR, JSL, BRK, COP, MVP, MVN
				_cpuBreakAddress = (_prevProgramCounter & 0xFF0000) | (((_prevProgramCounter & 0xFFFF) + DisassemblyInfo::GetOpSize(_prevOpCode, 0, CpuType::Cpu)) & 0xFFFF);
				_cpuStepCount = -1;
				_spcStepCount = -1;
				_ppuStepCount = -1;
				_spcBreakAddress = -1;
				_breakScanline = -1;
			} else {
				//For any other instruction, step over is the same as step into
				_cpuStepCount = 1;
				_spcStepCount = -1;
				_cpuBreakAddress = -1;
				_spcBreakAddress = -1;
				_ppuStepCount = -1;
				_breakScanline = -1;
			}
			break;

		case StepType::SpcStep:
			_spcStepCount = stepCount;
			_cpuStepCount = -1;
			_cpuBreakAddress = -1;
			_spcBreakAddress = -1;
			_ppuStepCount = -1;
			_breakScanline = -1;
			break;

		case StepType::SpcStepOut:
			_spcBreakAddress = _spcCallstackManager->GetReturnAddress();
			_spcStepCount = -1;
			_cpuBreakAddress = -1;
			_cpuStepCount = -1;
			_ppuStepCount = -1;
			_breakScanline = -1;
			break;

		case StepType::SpcStepOver:
			if(_spcPrevOpCode == 0x3F || _spcPrevOpCode == 0x0F) {
				//JSR, BRK
				_spcBreakAddress = _spcPrevProgramCounter + DisassemblyInfo::GetOpSize(_spcPrevOpCode, 0, CpuType::Spc);
				_cpuStepCount = -1;
				_spcStepCount = -1;
				_ppuStepCount = -1;
				_cpuBreakAddress = -1;
				_breakScanline = -1;
			} else {
				//For any other instruction, step over is the same as step into
				_spcStepCount = 1;
				_cpuStepCount = -1;
				_cpuBreakAddress = -1;
				_spcBreakAddress = -1;
				_ppuStepCount = -1;
				_breakScanline = -1;
			}
			break;

		case StepType::PpuStep:
			_ppuStepCount = stepCount;
			_cpuStepCount = -1;
			_spcStepCount = -1;
			_cpuBreakAddress = -1;
			_spcBreakAddress = -1;
			_breakScanline = -1;
			break;

		case StepType::SpecificScanline:
			_breakScanline = stepCount;
			_ppuStepCount = -1;
			_cpuStepCount = -1;
			_spcStepCount = -1;
			_cpuBreakAddress = -1;
			_spcBreakAddress = -1;
			break;
	}
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

void Debugger::GetState(DebugState &state)
{
	state.MasterClock = _memoryManager->GetMasterClock();
	state.Cpu = _cpu->GetState();
	state.Ppu = _ppu->GetState();
	state.Spc = _spc->GetState();
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
	uint32_t prgRomSize = _console->GetCartridge()->DebugGetPrgRomSize();
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

shared_ptr<CallstackManager> Debugger::GetCallstackManager(CpuType cpuType)
{
	return cpuType == CpuType::Cpu ? _callstackManager : _spcCallstackManager;
}

shared_ptr<Console> Debugger::GetConsole()
{
	return _console;
}