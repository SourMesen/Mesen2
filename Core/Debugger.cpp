#include "stdafx.h"
#include "Debugger.h"
#include "DebugTypes.h"
#include "Console.h"
#include "Cpu.h"
#include "Ppu.h"
#include "BaseCartridge.h"
#include "MemoryManager.h"
#include "SoundMixer.h"
#include "NotificationManager.h"
#include "CpuTypes.h"
#include "DisassemblyInfo.h"
#include "TraceLogger.h"
#include "MemoryDumper.h"
#include "CodeDataLogger.h"
#include "Disassembler.h"
#include "BreakpointManager.h"
#include "PpuTools.h"
#include "EventManager.h"
#include "EventType.h"
#include "ExpressionEvaluator.h"
#include "../Utilities/HexUtilities.h"
#include "../Utilities/FolderUtilities.h"

Debugger::Debugger(shared_ptr<Console> console)
{
	_console = console;
	_cpu = console->GetCpu();
	_ppu = console->GetPpu();
	_memoryManager = console->GetMemoryManager();

	_watchExpEval.reset(new ExpressionEvaluator(this));
	_codeDataLogger.reset(new CodeDataLogger(console->GetCartridge()->DebugGetPrgRomSize()));
	_disassembler.reset(new Disassembler(console, _codeDataLogger));
	_traceLogger.reset(new TraceLogger(this, _console));
	_memoryDumper.reset(new MemoryDumper(_ppu, _memoryManager, console->GetCartridge()));
	_breakpointManager.reset(new BreakpointManager(this));
	_ppuTools.reset(new PpuTools(_console.get(), _ppu.get()));
	_eventManager.reset(new EventManager(this, _cpu.get(), _ppu.get()));
	
	_cpuStepCount = 0;
	_executionStopped = false;
	_breakRequestCount = 0;

	string cdlFile = FolderUtilities::CombinePath(FolderUtilities::GetDebuggerFolder(), FolderUtilities::GetFilename(_console->GetCartridge()->GetRomInfo().RomPath, false) + ".cdl");
	_codeDataLogger->LoadCdlFile(cdlFile);

	//TODO: Thread safety
	uint32_t prgRomSize = console->GetCartridge()->DebugGetPrgRomSize();
	AddressInfo addrInfo;
	addrInfo.Type = SnesMemoryType::PrgRom;
	for(uint32_t i = 0; i < prgRomSize; i++) {
		if(_codeDataLogger->IsCode(i)) {
			addrInfo.Address = (int32_t)i;
			i += _disassembler->BuildCache(addrInfo, _codeDataLogger->GetCpuFlags(i)) - 1;
		}
	}
}

Debugger::~Debugger()
{
	string cdlFile = FolderUtilities::CombinePath(FolderUtilities::GetDebuggerFolder(), FolderUtilities::GetFilename(_console->GetCartridge()->GetRomInfo().RomPath, false) + ".cdl");
	_codeDataLogger->SaveCdlFile(cdlFile);
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
				if(_prevOpCode == 0x20 || _prevOpCode == 0x5C || _prevOpCode == 0xDC || _prevOpCode == 0xFC) {
					flags |= CdlFlags::SubEntryPoint;
				}
				_codeDataLogger->SetFlags(addressInfo.Address, flags);
			}
			_disassembler->BuildCache(addressInfo, state.PS);
		}

		DebugState debugState;
		GetState(debugState);

		DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo);
		_traceLogger->Log(debugState, disInfo);

		_prevOpCode = value;

		if(_cpuStepCount > 0) {
			_cpuStepCount--;
		}

		/*if(value == 0x00 || value == 0xDB || value == 0x42) {
			//Break on BRK/STP/WDM
			_cpuStepCount = 0;
		}*/
	} else if(type == MemoryOperationType::ExecOperand) {
		if(addressInfo.Type == SnesMemoryType::PrgRom && addressInfo.Address >= 0) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Code | (state.PS & (CdlFlags::IndexMode8 | CdlFlags::MemoryMode8)));
		}
	} else {
		if(addressInfo.Type == SnesMemoryType::PrgRom && addressInfo.Address >= 0) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Data | (state.PS & (CdlFlags::IndexMode8 | CdlFlags::MemoryMode8)));
		}
	}

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

	ProcessBreakConditions(operation, addressInfo);
}

void Debugger::ProcessWorkRamRead(uint32_t addr, uint8_t value)
{
	AddressInfo addressInfo(addr, SnesMemoryType::WorkRam);
	//TODO Make this more flexible/accurate
	MemoryOperationInfo operation(0x7E0000 | addr, value, MemoryOperationType::Read);
	ProcessBreakConditions(operation, addressInfo);
}

void Debugger::ProcessWorkRamWrite(uint32_t addr, uint8_t value)
{
	AddressInfo addressInfo(addr, SnesMemoryType::WorkRam);
	//TODO Make this more flexible/accurate
	MemoryOperationInfo operation(0x7E0000 | addr, value, MemoryOperationType::Write);
	ProcessBreakConditions(operation, addressInfo);
}

void Debugger::ProcessPpuRead(uint16_t addr, uint8_t value, SnesMemoryType memoryType)
{
	AddressInfo addressInfo(addr, memoryType);
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Read);
	ProcessBreakConditions(operation, addressInfo);
}

void Debugger::ProcessPpuWrite(uint16_t addr, uint8_t value, SnesMemoryType memoryType)
{
	AddressInfo addressInfo(addr, memoryType);
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Write);
	ProcessBreakConditions(operation, addressInfo);
}

void Debugger::ProcessPpuCycle()
{
	uint16_t scanline = _ppu->GetState().Scanline;
	uint16_t cycle = _ppu->GetState().Cycle;
	_ppuTools->UpdateViewers(scanline, cycle);
}

void Debugger::ProcessBreakConditions(MemoryOperationInfo &operation, AddressInfo &addressInfo)
{
	if(_breakpointManager->CheckBreakpoint(operation, addressInfo)) {
		_cpuStepCount = 0;
	}

	if(_cpuStepCount == 0 || _breakRequestCount) {
		_console->GetSoundMixer()->StopAudio();
		_disassembler->Disassemble();

		_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::CodeBreak);
		_executionStopped = true;
		while(_cpuStepCount == 0 || _breakRequestCount) {
			std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(10));
		}
		_executionStopped = false;
	}
}

void Debugger::ProcessEvent(EventType type)
{
	switch(type) {
		case EventType::Nmi: _eventManager->AddEvent(DebugEventType::Nmi); break;
		case EventType::Irq: _eventManager->AddEvent(DebugEventType::Irq); break;
		case EventType::StartFrame: _eventManager->ClearFrameEvents(); break;
	}
}

int32_t Debugger::EvaluateExpression(string expression, EvalResultType &resultType, bool useCache)
{
	MemoryOperationInfo operationInfo { 0, 0, MemoryOperationType::Read };
	DebugState state;
	GetState(state);
	if(useCache) {
		return _watchExpEval->Evaluate(expression, state, resultType, operationInfo);
	} else {
		ExpressionEvaluator expEval(this);
		return expEval.Evaluate(expression, state, resultType, operationInfo);
	}
}

void Debugger::Run()
{
	_cpuStepCount = -1;
}

void Debugger::Step(int32_t stepCount)
{
	_cpuStepCount = stepCount;
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

void Debugger::GetState(DebugState &state)
{
	state.Cpu = _cpu->GetState();
	state.Ppu = _ppu->GetState();
}

shared_ptr<TraceLogger> Debugger::GetTraceLogger()
{
	return _traceLogger;
}

shared_ptr<MemoryDumper> Debugger::GetMemoryDumper()
{
	return _memoryDumper;
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

shared_ptr<Console> Debugger::GetConsole()
{
	return _console;
}