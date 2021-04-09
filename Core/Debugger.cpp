#include "stdafx.h"
#include "Debugger.h"
#include "DebugTypes.h"
#include "Emulator.h"
#include "IConsole.h"
#include "SNES/Cpu.h"
#include "SNES/Ppu.h"
#include "SNES/Spc.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/Coprocessors/GSU/Gsu.h"
#include "SNES/Coprocessors/CX4/Cx4.h"
#include "SNES/Coprocessors/DSP/NecDsp.h"
#include "Gameboy/Gameboy.h"
#include "SNES/Debugger/CpuDebugger.h"
#include "SNES/Debugger/SpcDebugger.h"
#include "SNES/Debugger/GsuDebugger.h"
#include "SNES/Debugger/NecDspDebugger.h"
#include "SNES/Debugger/Cx4Debugger.h"
#include "Gameboy/Debugger/GbDebugger.h"
#include "SNES/BaseCartridge.h"
#include "SNES/MemoryManager.h"
#include "EmuSettings.h"
#include "SoundMixer.h"
#include "MemoryMappings.h"
#include "NotificationManager.h"
#include "SNES/CpuTypes.h"
#include "DisassemblyInfo.h"
#include "TraceLogger.h"
#include "MemoryDumper.h"
#include "MemoryAccessCounter.h"
#include "CodeDataLogger.h"
#include "Disassembler.h"
#include "BreakpointManager.h"
#include "PpuTools.h"
#include "EventType.h"
#include "DebugBreakHelper.h"
#include "LabelManager.h"
#include "ScriptManager.h"
#include "CallstackManager.h"
#include "ExpressionEvaluator.h"
#include "SNES/InternalRegisters.h"
#include "SNES/AluMulDiv.h"
#include "SNES/DmaController.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbPpu.h"
#include "Gameboy/GameboyHeader.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/IpsPatcher.h"
#include "MemoryOperationType.h"
#include "NES/Debugger/NesDebugger.h"
#include "NES/NesTypes.h"
#include "BaseState.h"
#include "BaseEventManager.h"

Debugger::Debugger(Emulator* emu, IConsole* console)
{
	_executionStopped = true;

	_emu = emu;
	_console = console;
	_settings = _emu->GetSettings();

	_labelManager.reset(new LabelManager(this));
	
	_memoryDumper.reset(new MemoryDumper(this));
	_disassembler.reset(new Disassembler(console, this));
	_traceLogger.reset(new TraceLogger(this));
	_memoryAccessCounter.reset(new MemoryAccessCounter(this));
	_ppuTools.reset(new PpuTools(_emu));
	_scriptManager.reset(new ScriptManager(this));

	vector<CpuType> cpuTypes = _emu->GetCpuTypes();
	for(CpuType type : cpuTypes) {
		unique_ptr<IDebugger> &debugger = _debuggers[(int)type].Debugger;
		switch(type) {
			case CpuType::Cpu: debugger.reset(new CpuDebugger(this, CpuType::Cpu)); break;
			case CpuType::Spc: debugger.reset(new SpcDebugger(this)); break;
			case CpuType::NecDsp: debugger.reset(new NecDspDebugger(this)); break;
			case CpuType::Sa1: debugger.reset(new CpuDebugger(this, CpuType::Sa1)); break;
			case CpuType::Gsu: debugger.reset(new GsuDebugger(this)); break;
			case CpuType::Cx4: debugger.reset(new Cx4Debugger(this)); break;
			case CpuType::Gameboy: debugger.reset(new GbDebugger(this)); break;
			case CpuType::Nes: debugger.reset(new NesDebugger(this)); break;
			default: throw std::runtime_error("Unsupported CPU type");
		}

		_debuggers[(int)type].Evaluator.reset(new ExpressionEvaluator(this, type));
	}

	_breakRequestCount = 0;
	_suspendRequestCount = 0;

	//TODO
	/*string cdlFile = FolderUtilities::CombinePath(FolderUtilities::GetDebuggerFolder(), FolderUtilities::GetFilename(_emu->GetRomInfo().RomFile.GetFileName(), false) + ".cdl");
	GetCodeDataLogger(CpuType::Cpu)->LoadCdlFile(cdlFile, _settings->CheckDebuggerFlag(DebuggerFlags::AutoResetCdl), _emu->GetCrc32());*/

	RefreshCodeCache();

	if(_emu->IsPaused()) {
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
	//TODO
	/*bool hasGameboy = _debuggers[(int)CpuType::Gameboy].Debugger != nullptr;
	CpuType cpuType = hasGameboy ? CpuType::Gameboy : CpuType::Cpu;
	string cdlFile = FolderUtilities::CombinePath(FolderUtilities::GetDebuggerFolder(), FolderUtilities::GetFilename(_emu->GetRomInfo().RomFile.GetFileName(), false) + ".cdl");
	GetCodeDataLogger(cpuType)->SaveCdlFile(cdlFile, _emu->GetCrc32());*/

	while(_executionStopped) {
		Run();
	}
}

void Debugger::Reset()
{
	_memoryAccessCounter->ResetCounts();
	for(int i = 0; i <= (int)DebugUtilities::GetLastCpuType(); i++) {
		if(_debuggers[i].Debugger) {
			_debuggers[i].Debugger->Reset();
		}
	}
}

template<CpuType type>
void Debugger::ProcessMemoryRead(uint32_t addr, uint8_t value, MemoryOperationType opType)
{
	_debuggers[(int)type].Debugger->ProcessRead(addr, value, opType);
	
	if(_scriptManager->HasScript()) {
		_scriptManager->ProcessMemoryOperation(addr, value, opType, type);
	}
}

template<CpuType type>
void Debugger::ProcessMemoryWrite(uint32_t addr, uint8_t value, MemoryOperationType opType)
{
	_debuggers[(int)type].Debugger->ProcessWrite(addr, value, opType);
	
	if(_scriptManager->HasScript()) {
		_scriptManager->ProcessMemoryOperation(addr, value, opType, type);
	}
}

template<CpuType type>
void Debugger::ProcessPpuRead(uint16_t addr, uint8_t value, SnesMemoryType memoryType)
{
	AddressInfo addressInfo { addr, memoryType };
	MemoryOperationInfo operation { addr, value, MemoryOperationType::Read };
	
	BreakpointManager* bpManager = _debuggers[(int)type].Debugger->GetBreakpointManager();
	ProcessBreakConditions(false, bpManager, operation, addressInfo);

	_memoryAccessCounter->ProcessMemoryRead(addressInfo, _emu->GetMasterClock());
}

template<CpuType type>
void Debugger::ProcessPpuWrite(uint16_t addr, uint8_t value, SnesMemoryType memoryType)
{
	AddressInfo addressInfo { addr, memoryType };
	MemoryOperationInfo operation { addr, value, MemoryOperationType::Write };
	
	BreakpointManager* bpManager = _debuggers[(int)type].Debugger->GetBreakpointManager();
	ProcessBreakConditions(false, bpManager, operation, addressInfo);

	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _emu->GetMasterClock());
}

template<CpuType type>
void Debugger::ProcessPpuCycle()
{
	uint16_t scanline = 0;
	uint16_t cycle = 0;

	switch(type) {
		case CpuType::Cpu: 
		case CpuType::Gameboy: 
			_debuggers[(int)type].Debugger->ProcessPpuCycle(scanline, cycle);
			break;
	}

	_ppuTools->UpdateViewers(scanline, cycle, type);

	if(_breakRequestCount > 0) {
		SleepUntilResume(BreakSource::Unspecified);
	}
}

void Debugger::SleepUntilResume(BreakSource source, MemoryOperationInfo *operation, int breakpointId)
{
	if(_suspendRequestCount) {
		return;
	}

	_emu->GetSoundMixer()->StopAudio();
	_disassembler->Disassemble(CpuType::Cpu);
	_disassembler->Disassemble(CpuType::Spc);
	_disassembler->Disassemble(CpuType::Sa1);
	_disassembler->Disassemble(CpuType::Gsu);
	_disassembler->Disassemble(CpuType::NecDsp);
	_disassembler->Disassemble(CpuType::Cx4);
	_disassembler->Disassemble(CpuType::Nes);
	_disassembler->RefreshDisassembly(CpuType::Gameboy);

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
		_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::CodeBreak, &evt);
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
	_debuggers[(int)type].Debugger->ProcessInterrupt(originalPc, currentPc, forNmi);
	ProcessEvent(forNmi ? EventType::Nmi : EventType::Irq);
}

void Debugger::ProcessEvent(EventType type)
{
	_scriptManager->ProcessEvent(type);

	switch(type) {
		default: break;

		case EventType::StartFrame:
			_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::EventViewerRefresh, (void*)CpuType::Cpu);
			GetEventManager(CpuType::Cpu)->ClearFrameEvents();
			break;

		case EventType::GbStartFrame:
			if(_emu->GetConsoleType() == ConsoleType::Gameboy || _emu->GetConsoleType() == ConsoleType::GameboyColor) {
				_scriptManager->ProcessEvent(EventType::StartFrame);
			}
			_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::EventViewerRefresh, (void*)CpuType::Gameboy);
			GetEventManager(CpuType::Gameboy)->ClearFrameEvents();
			break;
		
		case EventType::GbEndFrame:
			if(_emu->GetConsoleType() == ConsoleType::Gameboy || _emu->GetConsoleType() == ConsoleType::GameboyColor) {
				_scriptManager->ProcessEvent(EventType::EndFrame);
			}
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
	BaseState& state = _debuggers[(int)cpuType].Debugger->GetState();
	if(useCache) {
		return _debuggers[(int)cpuType].Evaluator->Evaluate(expression, state, resultType, operationInfo);
	} else {
		ExpressionEvaluator expEval(this, cpuType);
		return expEval.Evaluate(expression, state, resultType, operationInfo);
	}
}

void Debugger::Run()
{
	for(int i = 0; i <= (int)DebugUtilities::GetLastCpuType(); i++) {
		if(_debuggers[i].Debugger) {
			_debuggers[i].Debugger->Run();
		}
	}
	_waitForBreakResume = false;
}

void Debugger::Step(CpuType cpuType, int32_t stepCount, StepType type)
{
	DebugBreakHelper helper(this);
	IDebugger* debugger = _debuggers[(int)cpuType].Debugger.get();

	if(debugger) {
		debugger->Step(stepCount, type);
	}

	for(int i = 0; i <= (int)DebugUtilities::GetLastCpuType(); i++) {
		if(_debuggers[i].Debugger && _debuggers[i].Debugger.get() != debugger) {
			_debuggers[i].Debugger->Run();
		}
	}

	_waitForBreakResume = false;
}

bool Debugger::IsExecutionStopped()
{
	return _executionStopped || _emu->IsThreadPaused();
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

void Debugger::GetState(BaseState &dstState, CpuType cpuType)
{
	BaseState& srcState = GetStateRef(cpuType);
	switch(cpuType) {
		case CpuType::Cpu: memcpy(&dstState, &srcState, sizeof(CpuState)); break;
		case CpuType::Spc: memcpy(&dstState, &srcState, sizeof(SpcState)); break;
		case CpuType::NecDsp: memcpy(&dstState, &srcState, sizeof(NecDspState)); break;
		case CpuType::Sa1: memcpy(&dstState, &srcState, sizeof(CpuState)); break;
		case CpuType::Gsu: memcpy(&dstState, &srcState, sizeof(GsuState)); break;
		case CpuType::Cx4: memcpy(&dstState, &srcState, sizeof(Cx4State)); break;
		case CpuType::Gameboy: memcpy(&dstState, &srcState, sizeof(GbCpuState)); break;
		case CpuType::Nes: memcpy(&dstState, &srcState, sizeof(NesCpuState)); break;
	}
}

BaseState& Debugger::GetStateRef(CpuType cpuType)
{
	return _debuggers[(int)cpuType].Debugger->GetState();
}

AddressInfo Debugger::GetAbsoluteAddress(AddressInfo relAddress)
{
	return _console->GetAbsoluteAddress(relAddress);
}

AddressInfo Debugger::GetRelativeAddress(AddressInfo absAddress, CpuType cpuType)
{
	return _console->GetRelativeAddress(absAddress, cpuType);
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
	
	vector<CpuType> cpuTypes = _emu->GetCpuTypes();
	for(CpuType type : _emu->GetCpuTypes()) {
		RebuildPrgCache(type);
	}
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
		bool hasGameboy = _debuggers[(int)CpuType::Gameboy].Debugger != nullptr;
		SnesMemoryType prgType = hasGameboy ? SnesMemoryType::GbPrgRom : SnesMemoryType::PrgRom;

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
	for(int i = 0; i <= (int)DebugUtilities::GetLastCpuType(); i++) {
		if(_debuggers[i].Debugger) {
			_debuggers[i].Debugger->GetBreakpointManager()->SetBreakpoints(breakpoints, length);
		}
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
	//TODO
	/*vector<uint8_t> output;
	RomInfo romInfo = _emu->GetRomInfo();
	Gameboy* gb = _cart->GetGameboy();

	vector<uint8_t> rom;
	if(gb) {
		uint8_t* prgRom = _memoryDumper->GetMemoryBuffer(SnesMemoryType::GbPrgRom);
		uint32_t prgRomSize = _memoryDumper->GetMemorySize(SnesMemoryType::GbPrgRom);
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
	}*/
}

shared_ptr<TraceLogger> Debugger::GetTraceLogger()
{
	return _traceLogger;
}

MemoryDumper* Debugger::GetMemoryDumper()
{
	return _memoryDumper.get();
}

shared_ptr<MemoryAccessCounter> Debugger::GetMemoryAccessCounter()
{
	return _memoryAccessCounter;
}

shared_ptr<CodeDataLogger> Debugger::GetCodeDataLogger(CpuType cpuType)
{
	if(_debuggers[(int)cpuType].Debugger) {
		return _debuggers[(int)cpuType].Debugger->GetCodeDataLogger();
	}
	throw std::runtime_error("GetCodeDataLogger() - Unsupported CPU type");
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
	if(_debuggers[(int)cpuType].Debugger) {
		return _debuggers[(int)cpuType].Debugger->GetEventManager();
	}
	throw std::runtime_error("GetEventManager() - Unsupported CPU type");
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
	if(_debuggers[(int)cpuType].Debugger) {
		_debuggers[(int)cpuType].Debugger->GetCallstackManager();
	}
	throw std::runtime_error("GetCallstackManager() - Unsupported CPU type");
}

IConsole* Debugger::GetConsole()
{
	return _console;
}

Emulator* Debugger::GetEmulator()
{
	return _emu;
}

shared_ptr<IAssembler> Debugger::GetAssembler(CpuType cpuType)
{
	if(_debuggers[(int)cpuType].Debugger) {
		_debuggers[(int)cpuType].Debugger->GetAssembler();
	}
	throw std::runtime_error("GetAssembler() - Unsupported CPU type");
}

template void Debugger::ProcessMemoryRead<CpuType::Cpu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Sa1>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Spc>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Gsu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::NecDsp>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Cx4>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Gameboy>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Nes>(uint32_t addr, uint8_t value, MemoryOperationType opType);

template void Debugger::ProcessMemoryWrite<CpuType::Cpu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryWrite<CpuType::Sa1>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryWrite<CpuType::Spc>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryWrite<CpuType::Gsu>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryWrite<CpuType::NecDsp>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryWrite<CpuType::Cx4>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryWrite<CpuType::Gameboy>(uint32_t addr, uint8_t value, MemoryOperationType opType);
template void Debugger::ProcessMemoryWrite<CpuType::Nes>(uint32_t addr, uint8_t value, MemoryOperationType opType);

template void Debugger::ProcessInterrupt<CpuType::Cpu>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Debugger::ProcessInterrupt<CpuType::Sa1>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Debugger::ProcessInterrupt<CpuType::Gameboy>(uint32_t originalPc, uint32_t currentPc, bool forNmi);

template void Debugger::ProcessPpuRead<CpuType::Cpu>(uint16_t addr, uint8_t value, SnesMemoryType memoryType);
template void Debugger::ProcessPpuRead<CpuType::Gameboy>(uint16_t addr, uint8_t value, SnesMemoryType memoryType);
template void Debugger::ProcessPpuRead<CpuType::Nes>(uint16_t addr, uint8_t value, SnesMemoryType memoryType);

template void Debugger::ProcessPpuWrite<CpuType::Cpu>(uint16_t addr, uint8_t value, SnesMemoryType memoryType);
template void Debugger::ProcessPpuWrite<CpuType::Gameboy>(uint16_t addr, uint8_t value, SnesMemoryType memoryType);
template void Debugger::ProcessPpuWrite<CpuType::Nes>(uint16_t addr, uint8_t value, SnesMemoryType memoryType);

template void Debugger::ProcessPpuCycle<CpuType::Cpu>();
template void Debugger::ProcessPpuCycle<CpuType::Gameboy>();
template void Debugger::ProcessPpuCycle<CpuType::Nes>();
