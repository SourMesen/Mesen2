#include "stdafx.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/MemoryDumper.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/CodeDataLogger.h"
#include "Debugger/Disassembler.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/PpuTools.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/LabelManager.h"
#include "Debugger/ScriptManager.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Debugger/BaseEventManager.h"
#include "Debugger/TraceLogFileSaver.h"
#include "SNES/Cpu.h"
#include "SNES/Ppu.h"
#include "SNES/Spc.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/Coprocessors/GSU/Gsu.h"
#include "SNES/Coprocessors/CX4/Cx4.h"
#include "SNES/Coprocessors/DSP/NecDsp.h"
#include "SNES/BaseCartridge.h"
#include "SNES/MemoryManager.h"
#include "SNES/MemoryMappings.h"
#include "SNES/Debugger/CpuDebugger.h"
#include "SNES/Debugger/SpcDebugger.h"
#include "SNES/Debugger/GsuDebugger.h"
#include "SNES/Debugger/NecDspDebugger.h"
#include "SNES/Debugger/Cx4Debugger.h"
#include "SNES/InternalRegisters.h"
#include "SNES/AluMulDiv.h"
#include "SNES/DmaController.h"
#include "SNES/CpuTypes.h"
#include "NES/Debugger/NesDebugger.h"
#include "NES/NesTypes.h"
#include "Gameboy/Debugger/GbDebugger.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbPpu.h"
#include "Gameboy/GameboyHeader.h"
#include "Shared/EmuSettings.h"
#include "Shared/Audio/SoundMixer.h"
#include "Shared/NotificationManager.h"
#include "Shared/BaseState.h"
#include "Shared/Emulator.h"
#include "Debugger/ITraceLogger.h"
#include "Shared/Interfaces/IConsole.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/Patches/IpsPatcher.h"
#include "MemoryOperationType.h"
#include "EventType.h"

uint64_t ITraceLogger::NextRowId = 0;

Debugger::Debugger(Emulator* emu, IConsole* console)
{
	_executionStopped = true;

	_emu = emu;
	_console = console;
	_settings = _emu->GetSettings();

	_consoleType = _emu->GetConsoleType();

	_labelManager.reset(new LabelManager(this));
	
	_memoryDumper.reset(new MemoryDumper(this));
	_disassembler.reset(new Disassembler(console, this));
	_memoryAccessCounter.reset(new MemoryAccessCounter(this));
	_scriptManager.reset(new ScriptManager(this));
	_traceLogSaver.reset(new TraceLogFileSaver());

	_cpuTypes = _emu->GetCpuTypes();
	_mainCpuType = _cpuTypes[0];
	for(CpuType type : _cpuTypes) {
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

	for(CpuType type : _cpuTypes) {
		_debuggers[(int)type].Debugger->Init();
		
	}

	string cdlFile = FolderUtilities::CombinePath(FolderUtilities::GetDebuggerFolder(), FolderUtilities::GetFilename(_emu->GetRomInfo().RomFile.GetFileName(), false) + ".cdl");
	CodeDataLogger* cdl = _debuggers[(int)_mainCpuType].Debugger->GetCodeDataLogger();
	if(cdl) {
		cdl->LoadCdlFile(cdlFile, _settings->CheckDebuggerFlag(DebuggerFlags::AutoResetCdl), _emu->GetCrc32());
	}

	_breakRequestCount = 0;
	_suspendRequestCount = 0;

	RefreshCodeCache();

	if(_emu->IsPaused()) {
		Step(_mainCpuType, 1, StepType::Step);
	}
	_executionStopped = false;
}

Debugger::~Debugger()
{
	Release();
}

void Debugger::Release()
{
	string cdlFile = FolderUtilities::CombinePath(FolderUtilities::GetDebuggerFolder(), FolderUtilities::GetFilename(_emu->GetRomInfo().RomFile.GetFileName(), false) + ".cdl");
	CodeDataLogger* cdl = _debuggers[(int)_mainCpuType].Debugger->GetCodeDataLogger();
	if(cdl) {
		cdl->SaveCdlFile(cdlFile, _emu->GetCrc32());
	}

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

template<CpuType type, typename DebuggerType>
DebuggerType* Debugger::GetDebugger()
{
	return (DebuggerType*)_debuggers[(int)type].Debugger.get();
}

template<CpuType type>
void Debugger::ProcessMemoryRead(uint32_t addr, uint8_t value, MemoryOperationType opType)
{
	switch(type) {
		case CpuType::Cpu: GetDebugger<type, CpuDebugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::Spc: GetDebugger<type, SpcDebugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::NecDsp: GetDebugger<type, NecDspDebugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::Sa1: GetDebugger<type, CpuDebugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::Gsu: GetDebugger<type, GsuDebugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::Cx4: GetDebugger<type, Cx4Debugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::Gameboy: GetDebugger<type, GbDebugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::Nes: GetDebugger<type, NesDebugger>()->ProcessRead(addr, value, opType); break;
	}
	
	if(_scriptManager->HasScript()) {
		_scriptManager->ProcessMemoryOperation(addr, value, opType, type);
	}
}

template<CpuType type>
void Debugger::ProcessMemoryWrite(uint32_t addr, uint8_t value, MemoryOperationType opType)
{
	switch(type) {
		case CpuType::Cpu: GetDebugger<type, CpuDebugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::Spc: GetDebugger<type, SpcDebugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::NecDsp: GetDebugger<type, NecDspDebugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::Sa1: GetDebugger<type, CpuDebugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::Gsu: GetDebugger<type, GsuDebugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::Cx4: GetDebugger<type, Cx4Debugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::Gameboy: GetDebugger<type, GbDebugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::Nes: GetDebugger<type, NesDebugger>()->ProcessWrite(addr, value, opType); break;
	}
	
	if(_scriptManager->HasScript()) {
		_scriptManager->ProcessMemoryOperation(addr, value, opType, type);
	}
}

template<CpuType type>
void Debugger::ProcessPpuRead(uint16_t addr, uint8_t value, SnesMemoryType memoryType)
{
	switch(type) {
		case CpuType::Cpu: GetDebugger<type, CpuDebugger>()->ProcessPpuRead(addr, value, memoryType); break;
		case CpuType::Gameboy: GetDebugger<type, GbDebugger>()->ProcessPpuRead(addr, value, memoryType); break;
		case CpuType::Nes: GetDebugger<type, NesDebugger>()->ProcessPpuRead(addr, value, memoryType); break;
		default: throw std::runtime_error("Invalid cpu type");
	}
}

template<CpuType type>
void Debugger::ProcessPpuWrite(uint16_t addr, uint8_t value, SnesMemoryType memoryType)
{
	switch(type) {
		case CpuType::Cpu: GetDebugger<type, CpuDebugger>()->ProcessPpuWrite(addr, value, memoryType); break;
		case CpuType::Gameboy: GetDebugger<type, GbDebugger>()->ProcessPpuWrite(addr, value, memoryType); break;
		case CpuType::Nes: GetDebugger<type, NesDebugger>()->ProcessPpuWrite(addr, value, memoryType); break;
		default: throw std::runtime_error("Invalid cpu type");
	}
}

template<CpuType type>
void Debugger::ProcessPpuCycle()
{
	switch(type) {
		case CpuType::Cpu: GetDebugger<type, CpuDebugger>()->ProcessPpuCycle(); break;
		case CpuType::Gameboy: GetDebugger<type, GbDebugger>()->ProcessPpuCycle(); break;
		case CpuType::Nes: GetDebugger<type, NesDebugger>()->ProcessPpuCycle(); break;
		default: throw std::runtime_error("Invalid cpu type");
	}
}

void Debugger::SleepUntilResume(BreakSource source, MemoryOperationInfo *operation, int breakpointId)
{
	if(_suspendRequestCount) {
		return;
	}

	_executionStopped = true;
	
	bool notificationSent = false;
	if(source != BreakSource::Unspecified || _breakRequestCount == 0) {
		_emu->GetSoundMixer()->StopAudio();

		//Only trigger code break event if the pause was caused by user action
		BreakEvent evt = {};
		evt.BreakpointId = breakpointId;
		evt.Source = source;
		if(operation) {
			evt.Operation = *operation;
		}
		_waitForBreakResume = true;
		_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::CodeBreak, &evt);
		notificationSent = true;
	}

	while((_waitForBreakResume && !_suspendRequestCount) || _breakRequestCount) {
		std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(_breakRequestCount ? 1 : 10));
	}

	if(notificationSent) {
		_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::DebuggerResumed);
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

		case EventType::StartFrame: {
			_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::EventViewerRefresh, (void*)_mainCpuType);
			BaseEventManager* evtMgr = GetEventManager(_mainCpuType);
			if(evtMgr) {
				evtMgr->ClearFrameEvents();
			}
			break;
		}

		case EventType::GbStartFrame:
			if(_consoleType == ConsoleType::Gameboy || _consoleType == ConsoleType::GameboyColor) {
				_scriptManager->ProcessEvent(EventType::StartFrame);
			}
			_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::EventViewerRefresh, (void*)CpuType::Gameboy);
			GetEventManager(CpuType::Gameboy)->ClearFrameEvents();
			break;
		
		case EventType::GbEndFrame:
			if(_consoleType == ConsoleType::Gameboy || _consoleType == ConsoleType::GameboyColor) {
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
	MemoryOperationInfo operationInfo { 0, 0, MemoryOperationType::Read, SnesMemoryType::Register };
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
	//TODO
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

void Debugger::GetCpuState(BaseState &dstState, CpuType cpuType)
{
	BaseState& srcState = GetCpuStateRef(cpuType);
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

void Debugger::SetCpuState(BaseState& srcState, CpuType cpuType)
{
	DebugBreakHelper helper(this);
	BaseState& dstState = GetCpuStateRef(cpuType);
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

BaseState& Debugger::GetCpuStateRef(CpuType cpuType)
{
	return _debuggers[(int)cpuType].Debugger->GetState();
}

void Debugger::GetPpuState(BaseState& state, CpuType cpuType)
{
	switch(cpuType) {
		case CpuType::Cpu:
		case CpuType::Spc:
		case CpuType::NecDsp:
		case CpuType::Sa1:
		case CpuType::Gsu:
		case CpuType::Cx4: {
			GetDebugger<CpuType::Cpu, CpuDebugger>()->GetPpuState(state);
			break;
		}

		case CpuType::Gameboy: {
			GetDebugger<CpuType::Gameboy, GbDebugger>()->GetPpuState(state);
			break;
		}

		case CpuType::Nes: {
			GetDebugger<CpuType::Nes, NesDebugger>()->GetPpuState(state);
			break;
		}
	}
}

void Debugger::SetPpuState(BaseState& state, CpuType cpuType)
{
	DebugBreakHelper helper(this);
	switch(cpuType) {
		case CpuType::Cpu:
		case CpuType::Spc:
		case CpuType::NecDsp:
		case CpuType::Sa1:
		case CpuType::Gsu:
		case CpuType::Cx4: {
			GetDebugger<CpuType::Cpu, CpuDebugger>()->SetPpuState(state);
			break;
		}

		case CpuType::Gameboy: {
			GetDebugger<CpuType::Gameboy, GbDebugger>()->SetPpuState(state);
			break;
		}

		case CpuType::Nes: {
			GetDebugger<CpuType::Nes, NesDebugger>()->SetPpuState(state);
			break;
		}
	}
}

void Debugger::GetConsoleState(BaseState& state, ConsoleType consoleType)
{
	_console->GetConsoleState(state, consoleType);
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
	CodeDataLogger* cdl = GetCodeDataLogger(cpuType);
	if(!cdl) {
		return;
	}

	uint32_t prgRomSize = cdl->GetPrgSize();
	AddressInfo addrInfo;
	addrInfo.Type = cdl->GetPrgMemoryType();
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
	CodeDataLogger* cdl = GetCodeDataLogger(cpuType);
	SnesMemoryType prgType = cdl->GetPrgMemoryType();
	if(memoryType == prgType) {
		cdl->GetCdlData(offset, length, cdlData);
	} else {
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

ITraceLogger* Debugger::GetTraceLogger(CpuType cpuType)
{
	if(_debuggers[(int)cpuType].Debugger) {
		return _debuggers[(int)cpuType].Debugger->GetTraceLogger();
	}
	return nullptr;
}

uint32_t Debugger::GetExecutionTrace(TraceRow output[], uint32_t startOffset, uint32_t maxLineCount)
{
	DebugBreakHelper helper(this);

	uint32_t offsetsByCpu[(int)DebugUtilities::GetLastCpuType() + 1] = {};

	vector<CpuType> cpuTypes = _emu->GetCpuTypes();

	uint32_t count = 0;
	int64_t lastRowId = ITraceLogger::NextRowId;
	while(count < maxLineCount) {
		bool added = false;
		for(CpuType cpuType : cpuTypes) {
			ITraceLogger* logger = GetTraceLogger(cpuType);
			if(logger) {
				uint32_t& offset = offsetsByCpu[(int)cpuType];
				int64_t rowId = logger->GetRowId(offset);
				if(rowId == -1 || rowId != lastRowId - 1) {
					continue;
				}

				lastRowId = rowId;

				if(startOffset > 0) {
					//Skip rows until the part the UI wants to display is reached
					startOffset--;
				} else {
					logger->GetExecutionTrace(output[count], offset);
					count++;
				}
				offset++;
				added = true;
				break;
			}
		}
		if(!added) {
			break;
		}
	}

	return count;
}

TraceLogFileSaver* Debugger::GetTraceLogFileSaver()
{
	return _traceLogSaver.get();
}

MemoryDumper* Debugger::GetMemoryDumper()
{
	return _memoryDumper.get();
}

MemoryAccessCounter* Debugger::GetMemoryAccessCounter()
{
	return _memoryAccessCounter.get();
}

CodeDataLogger* Debugger::GetCodeDataLogger(CpuType cpuType)
{
	if(_debuggers[(int)cpuType].Debugger) {
		return _debuggers[(int)cpuType].Debugger->GetCodeDataLogger();
	}
	throw std::runtime_error("GetCodeDataLogger() - Unsupported CPU type");
}

Disassembler* Debugger::GetDisassembler()
{
	return _disassembler.get();
}

PpuTools* Debugger::GetPpuTools(CpuType cpuType)
{
	if(_debuggers[(int)cpuType].Debugger) {
		return _debuggers[(int)cpuType].Debugger->GetPpuTools();
	}
	return nullptr;
}

BaseEventManager* Debugger::GetEventManager(CpuType cpuType)
{
	if(_debuggers[(int)cpuType].Debugger) {
		return _debuggers[(int)cpuType].Debugger->GetEventManager();
	}
	return nullptr;
}

LabelManager* Debugger::GetLabelManager()
{
	return _labelManager.get();
}

ScriptManager* Debugger::GetScriptManager()
{
	return _scriptManager.get();
}

CallstackManager* Debugger::GetCallstackManager(CpuType cpuType)
{
	if(_debuggers[(int)cpuType].Debugger) {
		return _debuggers[(int)cpuType].Debugger->GetCallstackManager();
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

IAssembler* Debugger::GetAssembler(CpuType cpuType)
{
	if(_debuggers[(int)cpuType].Debugger) {
		return _debuggers[(int)cpuType].Debugger->GetAssembler();
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
template void Debugger::ProcessInterrupt<CpuType::Nes>(uint32_t originalPc, uint32_t currentPc, bool forNmi);

template void Debugger::ProcessPpuRead<CpuType::Cpu>(uint16_t addr, uint8_t value, SnesMemoryType memoryType);
template void Debugger::ProcessPpuRead<CpuType::Gameboy>(uint16_t addr, uint8_t value, SnesMemoryType memoryType);
template void Debugger::ProcessPpuRead<CpuType::Nes>(uint16_t addr, uint8_t value, SnesMemoryType memoryType);

template void Debugger::ProcessPpuWrite<CpuType::Cpu>(uint16_t addr, uint8_t value, SnesMemoryType memoryType);
template void Debugger::ProcessPpuWrite<CpuType::Gameboy>(uint16_t addr, uint8_t value, SnesMemoryType memoryType);
template void Debugger::ProcessPpuWrite<CpuType::Nes>(uint16_t addr, uint8_t value, SnesMemoryType memoryType);

template void Debugger::ProcessPpuCycle<CpuType::Cpu>();
template void Debugger::ProcessPpuCycle<CpuType::Gameboy>();
template void Debugger::ProcessPpuCycle<CpuType::Nes>();
