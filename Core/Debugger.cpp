#include "stdafx.h"
#include "Debugger.h"
#include "DebugTypes.h"
#include "Console.h"
#include "Cpu.h"
#include "Ppu.h"
#include "Spc.h"
#include "Sa1.h"
#include "Gsu.h"
#include "Cx4.h"
#include "NecDsp.h"
#include "Gameboy.h"
#include "CpuDebugger.h"
#include "SpcDebugger.h"
#include "GsuDebugger.h"
#include "NecDspDebugger.h"
#include "Cx4Debugger.h"
#include "GbDebugger.h"
#include "BaseCartridge.h"
#include "MemoryManager.h"
#include "EmuSettings.h"
#include "SoundMixer.h"
#include "MemoryMappings.h"
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
#include "GbEventManager.h"
#include "EventType.h"
#include "DebugBreakHelper.h"
#include "LabelManager.h"
#include "ScriptManager.h"
#include "CallstackManager.h"
#include "ExpressionEvaluator.h"
#include "InternalRegisters.h"
#include "AluMulDiv.h"
#include "Assembler.h"
#include "Gameboy.h"
#include "GbPpu.h"
#include "GbAssembler.h"
#include "GameboyHeader.h"
#include "../Utilities/HexUtilities.h"
#include "../Utilities/FolderUtilities.h"
#include "../Utilities/IpsPatcher.h"

Debugger::Debugger(shared_ptr<Console> console)
{
	_console = console;
	_cpu = console->GetCpu();
	_ppu = console->GetPpu();
	_spc = console->GetSpc();
	_cart = console->GetCartridge();
	_settings = console->GetSettings();
	_memoryManager = console->GetMemoryManager();
	_dmaController = console->GetDmaController();
	_internalRegs = console->GetInternalRegisters();
	_gameboy = _cart->GetGameboy();

	_labelManager.reset(new LabelManager(this));
	_watchExpEval[(int)CpuType::Cpu].reset(new ExpressionEvaluator(this, CpuType::Cpu));
	_watchExpEval[(int)CpuType::Spc].reset(new ExpressionEvaluator(this, CpuType::Spc));
	_watchExpEval[(int)CpuType::Sa1].reset(new ExpressionEvaluator(this, CpuType::Sa1));
	_watchExpEval[(int)CpuType::Gsu].reset(new ExpressionEvaluator(this, CpuType::Gsu));
	_watchExpEval[(int)CpuType::NecDsp].reset(new ExpressionEvaluator(this, CpuType::NecDsp));
	_watchExpEval[(int)CpuType::Cx4].reset(new ExpressionEvaluator(this, CpuType::Cx4));
	_watchExpEval[(int)CpuType::Gameboy].reset(new ExpressionEvaluator(this, CpuType::Gameboy));

	_codeDataLogger.reset(new CodeDataLogger(_cart->DebugGetPrgRomSize(), CpuType::Cpu));
	_memoryDumper.reset(new MemoryDumper(this));
	_disassembler.reset(new Disassembler(console, _codeDataLogger, this));
	_traceLogger.reset(new TraceLogger(this, _console));
	_memoryAccessCounter.reset(new MemoryAccessCounter(this, console.get()));
	_ppuTools.reset(new PpuTools(_console.get(), _ppu.get()));
	_scriptManager.reset(new ScriptManager(this));

	if(_gameboy) {
		_gbDebugger.reset(new GbDebugger(this));
	}
	_cpuDebugger.reset(new CpuDebugger(this, CpuType::Cpu));
	_spcDebugger.reset(new SpcDebugger(this));
	if(_cart->GetSa1()) {
		_sa1Debugger.reset(new CpuDebugger(this, CpuType::Sa1));
	} else if(_cart->GetGsu()) {
		_gsuDebugger.reset(new GsuDebugger(this));
	} else if(_cart->GetDsp()) {
		_necDspDebugger.reset(new NecDspDebugger(this));
	} else if(_cart->GetCx4()) {
		_cx4Debugger.reset(new Cx4Debugger(this));
	}

	_step.reset(new StepRequest());

	_executionStopped = true;
	_breakRequestCount = 0;
	_suspendRequestCount = 0;

	CpuType cpuType = _gbDebugger ? CpuType::Gameboy : CpuType::Cpu;
	string cdlFile = FolderUtilities::CombinePath(FolderUtilities::GetDebuggerFolder(), FolderUtilities::GetFilename(_cart->GetRomInfo().RomFile.GetFileName(), false) + ".cdl");
	GetCodeDataLogger(cpuType)->LoadCdlFile(cdlFile, _settings->CheckDebuggerFlag(DebuggerFlags::AutoResetCdl), _cart->GetCrc32());

	RefreshCodeCache();

	if(_console->IsPaused()) {
		Step(CpuType::Cpu, 1, StepType::Step);
	}
	_executionStopped = false;
}

Debugger::~Debugger()
{
	Release();
}

void Debugger::Release()
{
	CpuType cpuType = _gbDebugger ? CpuType::Gameboy : CpuType::Cpu;
	string cdlFile = FolderUtilities::CombinePath(FolderUtilities::GetDebuggerFolder(), FolderUtilities::GetFilename(_cart->GetRomInfo().RomFile.GetFileName(), false) + ".cdl");
	GetCodeDataLogger(cpuType)->SaveCdlFile(cdlFile, _cart->GetCrc32());

	while(_executionStopped) {
		Run();
	}
}

void Debugger::Reset()
{
	_memoryAccessCounter->ResetCounts();
	_cpuDebugger->Reset();
	_spcDebugger->Reset();
	if(_sa1Debugger) {
		_sa1Debugger->Reset();
	}
}

template<CpuType type>
void Debugger::ProcessMemoryRead(uint32_t addr, uint8_t value, MemoryOperationType opType)
{
	switch(type) {
		case CpuType::Cpu: _cpuDebugger->ProcessRead(addr, value, opType); break;
		case CpuType::Spc: _spcDebugger->ProcessRead(addr, value, opType); break;
		case CpuType::NecDsp: _necDspDebugger->ProcessRead(addr, value, opType); break;
		case CpuType::Sa1: _sa1Debugger->ProcessRead(addr, value, opType); break;
		case CpuType::Gsu: _gsuDebugger->ProcessRead(addr, value, opType); break;
		case CpuType::Cx4: _cx4Debugger->ProcessRead(addr, value, opType); break;
		case CpuType::Gameboy: _gbDebugger->ProcessRead(addr, value, opType); break;
	}
	
	if(_scriptManager->HasScript()) {
		_scriptManager->ProcessMemoryOperation(addr, value, opType, type);
	}
}

template<CpuType type>
void Debugger::ProcessMemoryWrite(uint32_t addr, uint8_t value, MemoryOperationType opType)
{
	switch(type) {
		case CpuType::Cpu: _cpuDebugger->ProcessWrite(addr, value, opType); break;
		case CpuType::Spc: _spcDebugger->ProcessWrite(addr, value, opType); break;
		case CpuType::NecDsp: _necDspDebugger->ProcessWrite(addr, value, opType); break;
		case CpuType::Sa1: _sa1Debugger->ProcessWrite(addr, value, opType); break;
		case CpuType::Gsu: _gsuDebugger->ProcessWrite(addr, value, opType); break;
		case CpuType::Cx4: _cx4Debugger->ProcessWrite(addr, value, opType); break;
		case CpuType::Gameboy: _gbDebugger->ProcessWrite(addr, value, opType); break;
	}
	
	if(_scriptManager->HasScript()) {
		_scriptManager->ProcessMemoryOperation(addr, value, opType, type);
	}
}

void Debugger::ProcessWorkRamRead(uint32_t addr, uint8_t value)
{
	AddressInfo addressInfo { (int32_t)addr, SnesMemoryType::WorkRam };
	_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());
	
	MemoryOperationInfo operation { 0x7E0000 | addr, value, MemoryOperationType::Read };
	ProcessBreakConditions(false, _cpuDebugger->GetBreakpointManager(), operation, addressInfo);
}

void Debugger::ProcessWorkRamWrite(uint32_t addr, uint8_t value)
{
	AddressInfo addressInfo { (int32_t)addr, SnesMemoryType::WorkRam };
	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetMasterClock());
	
	MemoryOperationInfo operation { 0x7E0000 | addr, value, MemoryOperationType::Write };
	ProcessBreakConditions(false, _cpuDebugger->GetBreakpointManager(), operation, addressInfo);
}

void Debugger::ProcessPpuRead(uint16_t addr, uint8_t value, SnesMemoryType memoryType)
{
	AddressInfo addressInfo { addr, memoryType };
	MemoryOperationInfo operation { addr, value, MemoryOperationType::Read };
	ProcessBreakConditions(false, _cpuDebugger->GetBreakpointManager(), operation, addressInfo);

	_memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());
}

void Debugger::ProcessPpuWrite(uint16_t addr, uint8_t value, SnesMemoryType memoryType)
{
	AddressInfo addressInfo { addr, memoryType };
	MemoryOperationInfo operation { addr, value, MemoryOperationType::Write };
	ProcessBreakConditions(false, _cpuDebugger->GetBreakpointManager(), operation, addressInfo);

	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetMasterClock());
}

template<CpuType cpuType>
void Debugger::ProcessPpuCycle()
{
	uint16_t scanline;
	uint16_t cycle;
	if(cpuType == CpuType::Gameboy) {
		scanline = _gameboy->GetPpu()->GetScanline();
		cycle = _gameboy->GetPpu()->GetCycle();
	} else {
		scanline = _ppu->GetScanline();
		cycle = _memoryManager->GetHClock();
	}

	_ppuTools->UpdateViewers(scanline, cycle, cpuType);

	switch(cpuType) {
		case CpuType::Cpu: 
			//Catch up SPC/DSP as needed (if we're tracing or debugging those particular CPUs)
			if(_traceLogger->IsCpuLogged(CpuType::Spc) || _settings->CheckDebuggerFlag(DebuggerFlags::SpcDebuggerEnabled)) {
				_spc->Run();
			} else if(_traceLogger->IsCpuLogged(CpuType::NecDsp)) {
				_cart->RunCoprocessors();
			}

			_cpuDebugger->ProcessPpuCycle(scanline, cycle);
			break;

		case CpuType::Gameboy: 
			_gbDebugger->ProcessPpuCycle(scanline, cycle);
			break;
	}

	if(_breakRequestCount > 0) {
		SleepUntilResume(BreakSource::Unspecified);
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
	if(_cart->GetSa1()) {
		_disassembler->Disassemble(CpuType::Sa1);
	} else if(_cart->GetGsu()) {
		_disassembler->Disassemble(CpuType::Gsu);
	} else if(_cart->GetDsp()) {
		_disassembler->Disassemble(CpuType::NecDsp);
	} else if(_cart->GetCx4()) {
		_disassembler->Disassemble(CpuType::Cx4);
	} else if(_cart->GetGameboy()) {
		_disassembler->RefreshDisassembly(CpuType::Gameboy);
	}

	_executionStopped = true;
	
	if(source != BreakSource::Unspecified || _breakRequestCount == 0) {
		//Only trigger code break event if the pause was caused by user action
		BreakEvent evt = {};
		evt.BreakpointId = breakpointId;
		evt.Source = source;
		if(operation) {
			evt.Operation = *operation;
		}
		_waitForBreakResume = true;
		_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::CodeBreak, &evt);
	}

	while((_waitForBreakResume && !_suspendRequestCount) || _breakRequestCount) {
		std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(10));
	}

	_executionStopped = false;
}

void Debugger::ProcessBreakConditions(bool needBreak, BreakpointManager* bpManager, MemoryOperationInfo &operation, AddressInfo &addressInfo, BreakSource source)
{
	if(needBreak || _breakRequestCount || _waitForBreakResume) {
		SleepUntilResume(source);
	} else {
		int breakpointId = bpManager->CheckBreakpoint(operation, addressInfo);
		if(breakpointId >= 0) {
			SleepUntilResume(BreakSource::Breakpoint, &operation, breakpointId);
		}
	}
}

template<CpuType type>
void Debugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	switch(type) {
		case CpuType::Cpu: _cpuDebugger->ProcessInterrupt(originalPc, currentPc, forNmi); break;
		case CpuType::Sa1: _sa1Debugger->ProcessInterrupt(originalPc, currentPc, forNmi); break;
		case CpuType::Gameboy: _gbDebugger->ProcessInterrupt(originalPc, currentPc); break;
	}

	ProcessEvent(forNmi ? EventType::Nmi : EventType::Irq);
}

void Debugger::ProcessEvent(EventType type)
{
	_scriptManager->ProcessEvent(type);

	switch(type) {
		default: break;

		case EventType::StartFrame:
			_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::EventViewerRefresh, (void*)CpuType::Cpu);
			GetEventManager(CpuType::Cpu)->ClearFrameEvents();
			break;

		case EventType::GbStartFrame:
			_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::EventViewerRefresh, (void*)CpuType::Gameboy);
			GetEventManager(CpuType::Gameboy)->ClearFrameEvents();
			break;

		case EventType::Reset:
			Reset();
			break;

		case EventType::StateLoaded:
			_memoryAccessCounter->ResetCounts();
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
	_cpuDebugger->Run();
	_spcDebugger->Run();
	if(_sa1Debugger) {
		_sa1Debugger->Run();
	}
	if(_gsuDebugger) {
		_gsuDebugger->Run();
	}
	if(_necDspDebugger) {
		_necDspDebugger->Run();
	}
	if(_cx4Debugger) {
		_cx4Debugger->Run();
	}	
	if(_gbDebugger) {
		_gbDebugger->Run();
	}
	_waitForBreakResume = false;
}

void Debugger::Step(CpuType cpuType, int32_t stepCount, StepType type)
{
	DebugBreakHelper helper(this);
	StepRequest step;
	IDebugger *debugger = nullptr;

	switch(cpuType) {
		case CpuType::Cpu: debugger = _cpuDebugger.get(); break;
		case CpuType::Spc: debugger = _spcDebugger.get(); break;
		case CpuType::NecDsp: debugger = _necDspDebugger.get(); break;
		case CpuType::Sa1: debugger = _sa1Debugger.get(); break;
		case CpuType::Gsu: debugger = _gsuDebugger.get(); break;
		case CpuType::Cx4: debugger = _cx4Debugger.get(); break;
		case CpuType::Gameboy: debugger = _gbDebugger.get(); break;
	}
	debugger->Step(stepCount, type);
	
	if(!debugger) {
		_step.reset(new StepRequest(step));
	}
	if(debugger != _cpuDebugger.get()) {
		_cpuDebugger->Run();
	}
	if(debugger != _spcDebugger.get()) {
		_spcDebugger->Run();
	}
	if(_sa1Debugger && debugger != _sa1Debugger.get()) {
		_sa1Debugger->Run();
	}
	if(_gsuDebugger && debugger != _gsuDebugger.get()) {
		_gsuDebugger->Run();
	}
	if(_necDspDebugger && debugger != _necDspDebugger.get()) {
		_necDspDebugger->Run();
	}
	if(_cx4Debugger && debugger != _cx4Debugger.get()) {
		_cx4Debugger->Run();
	}
	if(_gbDebugger && debugger != _gbDebugger.get()) {
		_gbDebugger->Run();
	}
	_waitForBreakResume = false;
}

bool Debugger::IsExecutionStopped()
{
	return _executionStopped || _console->IsThreadPaused();
}

bool Debugger::HasBreakRequest()
{
	return _breakRequestCount > 0;
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
		if(_suspendRequestCount > 0) {
			_suspendRequestCount--;
		} else {
		#ifdef _DEBUG
			//throw std::runtime_error("unexpected debugger suspend::release call");
		#endif
		}
	} else {
		_suspendRequestCount++;
	}
}

void Debugger::BreakImmediately(BreakSource source)
{
	bool gbDebugger = _settings->CheckDebuggerFlag(DebuggerFlags::GbDebuggerEnabled);
	if(source == BreakSource::GbDisableLcdOutsideVblank && (!gbDebugger || !_settings->CheckDebuggerFlag(DebuggerFlags::GbBreakOnDisableLcdOutsideVblank))) {
		return;
	} else if(source == BreakSource::GbInvalidVramAccess && (!gbDebugger || !_settings->CheckDebuggerFlag(DebuggerFlags::GbBreakOnInvalidVramAccess))) {
		return;
	} else if(source == BreakSource::GbInvalidOamAccess && (!gbDebugger || !_settings->CheckDebuggerFlag(DebuggerFlags::GbBreakOnInvalidOamAccess))) {
		return;
	}
	SleepUntilResume(source);
}

void Debugger::GetState(DebugState &state, bool partialPpuState)
{
	state.MasterClock = _memoryManager->GetMasterClock();
	state.Cpu = _cpu->GetState();
	_ppu->GetState(state.Ppu, partialPpuState);
	state.Spc = _spc->GetState();
	state.Dsp = _spc->GetDspState();

	if(!partialPpuState) {
		for(int i = 0; i < 8; i++) {
			state.DmaChannels[i] = _dmaController->GetChannelConfig(i);
		}
		state.InternalRegs = _internalRegs->GetState();
		state.Alu = _internalRegs->GetAluState();
	}

	if(_cart->GetDsp()) {
		state.NecDsp = _cart->GetDsp()->GetState();
	}
	if(_cart->GetSa1()) {
		state.Sa1 = _cart->GetSa1()->GetState();
	}
	if(_cart->GetGsu()) {
		state.Gsu = _cart->GetGsu()->GetState();
	}
	if(_cart->GetCx4()) {
		state.Cx4 = _cart->GetCx4()->GetState();
	}
	if(_cart->GetGameboy()) {
		state.Gameboy = _cart->GetGameboy()->GetState();
	}
}

AddressInfo Debugger::GetAbsoluteAddress(AddressInfo relAddress)
{
	if(relAddress.Type == SnesMemoryType::CpuMemory) {
		if(_memoryManager->IsRegister(relAddress.Address)) {
			return { relAddress.Address & 0xFFFF, SnesMemoryType::Register };
		} else {
			return _memoryManager->GetMemoryMappings()->GetAbsoluteAddress(relAddress.Address);
		}
	} else if(relAddress.Type == SnesMemoryType::SpcMemory) {
		return _spc->GetAbsoluteAddress(relAddress.Address);
	} else if(relAddress.Type == SnesMemoryType::Sa1Memory) {
		return _cart->GetSa1()->GetMemoryMappings()->GetAbsoluteAddress(relAddress.Address);
	} else if(relAddress.Type == SnesMemoryType::GsuMemory) {
		return _cart->GetGsu()->GetMemoryMappings()->GetAbsoluteAddress(relAddress.Address);
	} else if(relAddress.Type == SnesMemoryType::Cx4Memory) {
		return _cart->GetCx4()->GetMemoryMappings()->GetAbsoluteAddress(relAddress.Address);
	} else if(relAddress.Type == SnesMemoryType::NecDspMemory) {
		return { relAddress.Address, SnesMemoryType::DspProgramRom };
	} else if(relAddress.Type == SnesMemoryType::GameboyMemory) {
		return _cart->GetGameboy()->GetAbsoluteAddress(relAddress.Address);
	}

	throw std::runtime_error("Unsupported address type");
}

AddressInfo Debugger::GetRelativeAddress(AddressInfo absAddress, CpuType cpuType)
{
	MemoryMappings* mappings = nullptr;
	switch(cpuType) {
		case CpuType::Cpu: mappings = _memoryManager->GetMemoryMappings(); break;
		case CpuType::Spc: break;
		case CpuType::NecDsp: break;
		case CpuType::Sa1: mappings = _cart->GetSa1()->GetMemoryMappings(); break;
		case CpuType::Gsu: mappings = _cart->GetGsu()->GetMemoryMappings(); break;
		case CpuType::Cx4: mappings = _cart->GetCx4()->GetMemoryMappings(); break;
		case CpuType::Gameboy: break;
	}

	switch(absAddress.Type) {
		case SnesMemoryType::PrgRom:
		case SnesMemoryType::WorkRam:
		case SnesMemoryType::SaveRam: {
			if(!mappings) {
				throw std::runtime_error("Unsupported cpu type");
			}

			uint8_t startBank = 0;
			//Try to find a mirror close to where the PC is
			if(cpuType == CpuType::Cpu) {
				if(absAddress.Type == SnesMemoryType::WorkRam) {
					startBank = 0x7E;
				} else {
					startBank = _cpu->GetState().K & 0xC0;
				}
			} else if(cpuType == CpuType::Sa1) {
				startBank = (_cart->GetSa1()->GetCpuState().K & 0xC0);
			} else if(cpuType == CpuType::Gsu) {
				startBank = (_cart->GetGsu()->GetState().ProgramBank & 0xC0);
			}

			return { mappings->GetRelativeAddress(absAddress, startBank), DebugUtilities::GetCpuMemoryType(cpuType) };
		}

		case SnesMemoryType::SpcRam:
		case SnesMemoryType::SpcRom:
			return { _spc->GetRelativeAddress(absAddress), SnesMemoryType::SpcMemory };
		
		case SnesMemoryType::GbPrgRom:
		case SnesMemoryType::GbWorkRam:
		case SnesMemoryType::GbCartRam:
		case SnesMemoryType::GbHighRam:
		case SnesMemoryType::GbBootRom:
			return { _cart->GetGameboy()->GetRelativeAddress(absAddress), SnesMemoryType::GameboyMemory };

		case SnesMemoryType::DspProgramRom:
			return { absAddress.Address, SnesMemoryType::NecDspMemory };

		case SnesMemoryType::Register:
			return { absAddress.Address & 0xFFFF, SnesMemoryType::Register };

		default: 
			return { -1, SnesMemoryType::Register };
	}
}

void Debugger::SetCdlData(CpuType cpuType, uint8_t *cdlData, uint32_t length)
{
	DebugBreakHelper helper(this);
	GetCodeDataLogger(cpuType)->SetCdlData(cdlData, length);
	RefreshCodeCache();
}

void Debugger::MarkBytesAs(CpuType cpuType, uint32_t start, uint32_t end, uint8_t flags)
{
	DebugBreakHelper helper(this);
	GetCodeDataLogger(cpuType)->MarkBytesAs(start, end, flags);
	RefreshCodeCache();
}

void Debugger::RefreshCodeCache()
{
	_disassembler->ResetPrgCache();
	RebuildPrgCache(CpuType::Cpu);
	RebuildPrgCache(CpuType::Gameboy);
}

void Debugger::RebuildPrgCache(CpuType cpuType)
{
	shared_ptr<CodeDataLogger> cdl = GetCodeDataLogger(cpuType);
	if(!cdl) {
		return;
	}

	uint32_t prgRomSize = cdl->GetPrgSize();
	AddressInfo addrInfo;
	addrInfo.Type = cpuType == CpuType::Gameboy ? SnesMemoryType::GbPrgRom : SnesMemoryType::PrgRom;

	for(uint32_t i = 0; i < prgRomSize; i++) {
		if(cdl->IsCode(i)) {
			addrInfo.Address = (int32_t)i;
			i += _disassembler->BuildCache(addrInfo, cdl->GetCpuFlags(i), cdl->GetCpuType(i)) - 1;
		}
	}
}

void Debugger::GetCdlData(uint32_t offset, uint32_t length, SnesMemoryType memoryType, uint8_t* cdlData)
{
	CpuType cpuType = DebugUtilities::ToCpuType(memoryType);
	shared_ptr<CodeDataLogger> cdl = GetCodeDataLogger(cpuType);
	if(memoryType == SnesMemoryType::PrgRom || memoryType == SnesMemoryType::GbPrgRom) {
		cdl->GetCdlData(offset, length, cdlData);
	} else {
		SnesMemoryType prgType = _gbDebugger ? SnesMemoryType::GbPrgRom : SnesMemoryType::PrgRom;

		AddressInfo relAddress;
		relAddress.Type = memoryType;
		for(uint32_t i = 0; i < length; i++) {
			relAddress.Address = offset + i;
			AddressInfo info = GetAbsoluteAddress(relAddress);
			cdlData[i] = info.Type == prgType ? cdl->GetFlags(info.Address) : 0;
		}
	}
}

void Debugger::SetBreakpoints(Breakpoint breakpoints[], uint32_t length)
{
	_cpuDebugger->GetBreakpointManager()->SetBreakpoints(breakpoints, length);
	_spcDebugger->GetBreakpointManager()->SetBreakpoints(breakpoints, length);
	if(_gsuDebugger) {
		_gsuDebugger->GetBreakpointManager()->SetBreakpoints(breakpoints, length);
	}
	if(_sa1Debugger) {
		_sa1Debugger->GetBreakpointManager()->SetBreakpoints(breakpoints, length);
	}
	if(_necDspDebugger) {
		_necDspDebugger->GetBreakpointManager()->SetBreakpoints(breakpoints, length);
	}
	if(_cx4Debugger) {
		_cx4Debugger->GetBreakpointManager()->SetBreakpoints(breakpoints, length);
	}
	if(_gbDebugger) {
		_gbDebugger->GetBreakpointManager()->SetBreakpoints(breakpoints, length);
	}
}

void Debugger::Log(string message)
{
	auto lock = _logLock.AcquireSafe();
	if(_debuggerLog.size() >= 1000) {
		_debuggerLog.pop_front();
	}
	_debuggerLog.push_back(message);
}

string Debugger::GetLog()
{
	auto lock = _logLock.AcquireSafe();
	stringstream ss;
	for(string& msg : _debuggerLog) {
		ss << msg << "\n";
	}
	return ss.str();
}

void Debugger::SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption)
{
	vector<uint8_t> output;
	RomInfo romInfo = _cart->GetRomInfo();
	Gameboy* gb = _cart->GetGameboy();

	vector<uint8_t> rom;
	if(gb) {
		uint8_t* prgRom = gb->DebugGetMemory(SnesMemoryType::GbPrgRom);
		uint32_t prgRomSize = gb->DebugGetMemorySize(SnesMemoryType::GbPrgRom);
		rom = vector<uint8_t>(prgRom, prgRom+prgRomSize);
	} else {
		rom = vector<uint8_t>(_cart->DebugGetPrgRom(), _cart->DebugGetPrgRom() + _cart->DebugGetPrgRomSize());
	}

	if(saveAsIps) {
		output = IpsPatcher::CreatePatch(_cart->GetOriginalPrgRom(), rom);
	} else {
		if(stripOption != CdlStripOption::StripNone) {
			GetCodeDataLogger(gb ? CpuType::Gameboy : CpuType::Cpu)->StripData(rom.data(), stripOption);

			//Preserve rom header regardless of CDL file contents
			if(gb) {
				GameboyHeader header = gb->GetHeader();
				memcpy(rom.data() + romInfo.HeaderOffset, &header, sizeof(GameboyHeader));
			} else {
				memcpy(rom.data() + romInfo.HeaderOffset, &romInfo.Header, sizeof(SnesCartInformation));
			}
		}
		output = rom;
	}

	ofstream file(filename, ios::out | ios::binary);
	if(file) {
		file.write((char*)output.data(), output.size());
		file.close();
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

shared_ptr<CodeDataLogger> Debugger::GetCodeDataLogger(CpuType cpuType)
{
	if(cpuType == CpuType::Gameboy) {
		return _gbDebugger ? _gbDebugger->GetCodeDataLogger() : nullptr;
	} else {
		return _codeDataLogger;
	}
}

shared_ptr<Disassembler> Debugger::GetDisassembler()
{
	return _disassembler;
}

shared_ptr<PpuTools> Debugger::GetPpuTools()
{
	return _ppuTools;
}

shared_ptr<IEventManager> Debugger::GetEventManager(CpuType cpuType)
{
	if(cpuType == CpuType::Gameboy) {
		return std::dynamic_pointer_cast<IEventManager>(_gbDebugger->GetEventManager());
	} else {
		return std::dynamic_pointer_cast<IEventManager>(_cpuDebugger->GetEventManager());
	}
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
	switch(cpuType) {
		case CpuType::Cpu: return _cpuDebugger->GetCallstackManager();
		case CpuType::Spc: return _spcDebugger->GetCallstackManager();
		case CpuType::Sa1: return _sa1Debugger->GetCallstackManager();
		case CpuType::Gameboy: return _gbDebugger->GetCallstackManager(); break;

		case CpuType::Gsu:
		case CpuType::NecDsp:
		case CpuType::Cx4:
			break;
	}
	throw std::runtime_error("GetCallstackManager() - Unsupported CPU type");
}

shared_ptr<Console> Debugger::GetConsole()
{
	return _console;
}

shared_ptr<IAssembler> Debugger::GetAssembler(CpuType cpuType)
{
	if(cpuType == CpuType::Gameboy) {
		return std::dynamic_pointer_cast<IAssembler>(_gbDebugger->GetAssembler());
	} else {
		return std::dynamic_pointer_cast<IAssembler>(_cpuDebugger->GetAssembler());
	}
}

template void Debugger::ProcessMemoryRead<CpuType::Cpu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Sa1>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Spc>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Gsu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::NecDsp>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Cx4>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Gameboy>(uint32_t addr, uint8_t value, MemoryOperationType opType);

template void Debugger::ProcessMemoryWrite<CpuType::Cpu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryWrite<CpuType::Sa1>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryWrite<CpuType::Spc>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryWrite<CpuType::Gsu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryWrite<CpuType::NecDsp>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryWrite<CpuType::Cx4>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryWrite<CpuType::Gameboy>(uint32_t addr, uint8_t value, MemoryOperationType opType);

template void Debugger::ProcessInterrupt<CpuType::Cpu>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Debugger::ProcessInterrupt<CpuType::Sa1>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Debugger::ProcessInterrupt<CpuType::Gameboy>(uint32_t originalPc, uint32_t currentPc, bool forNmi);

template void Debugger::ProcessPpuCycle<CpuType::Cpu>();
template void Debugger::ProcessPpuCycle<CpuType::Gameboy>();
