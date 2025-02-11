#include "pch.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/MemoryDumper.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/CodeDataLogger.h"
#include "Debugger/Disassembler.h"
#include "Debugger/DisassemblySearch.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/PpuTools.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/LabelManager.h"
#include "Debugger/ScriptManager.h"
#include "Debugger/ScriptHost.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Debugger/BaseEventManager.h"
#include "Debugger/TraceLogFileSaver.h"
#include "Debugger/CdlManager.h"
#include "Debugger/ITraceLogger.h"
#include "SNES/SnesCpuTypes.h"
#include "SNES/SpcTypes.h"
#include "SNES/Coprocessors/SA1/Sa1Types.h"
#include "SNES/Coprocessors/GSU/GsuTypes.h"
#include "SNES/Coprocessors/CX4/Cx4Types.h"
#include "SNES/Coprocessors/DSP/NecDspTypes.h"
#include "SNES/Coprocessors/ST018/ArmV3Types.h"
#include "SNES/Debugger/SnesDebugger.h"
#include "SNES/Debugger/SpcDebugger.h"
#include "SNES/Debugger/GsuDebugger.h"
#include "SNES/Debugger/St018Debugger.h"
#include "SNES/Debugger/NecDspDebugger.h"
#include "SNES/Debugger/Cx4Debugger.h"
#include "NES/Debugger/NesDebugger.h"
#include "NES/NesTypes.h"
#include "Gameboy/Debugger/GbDebugger.h"
#include "Gameboy/GbTypes.h"
#include "PCE/Debugger/PceDebugger.h"
#include "PCE/PceTypes.h"
#include "SMS/Debugger/SmsDebugger.h"
#include "SMS/SmsTypes.h"
#include "GBA/Debugger/GbaDebugger.h"
#include "GBA/GbaTypes.h"
#include "WS/Debugger/WsDebugger.h"
#include "WS/WsTypes.h"
#include "Shared/BaseControlManager.h"
#include "Shared/EmuSettings.h"
#include "Shared/Audio/SoundMixer.h"
#include "Shared/NotificationManager.h"
#include "Shared/BaseState.h"
#include "Shared/Emulator.h"
#include "Shared/Interfaces/IConsole.h"
#include "Shared/MemoryOperationType.h"
#include "Shared/EventType.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/Patches/IpsPatcher.h"
#include "Utilities/PlatformUtilities.h"

uint64_t ITraceLogger::NextRowId = 0;

Debugger::Debugger(Emulator* emu, IConsole* console)
{
	_executionStopped = true;

	_emu = emu;
	_console = console;
	_settings = _emu->GetSettings();

	_consoleType = _emu->GetConsoleType();

	vector<CpuType> cpuTypes = _emu->GetCpuTypes();
	_cpuTypes = unordered_set<CpuType>(cpuTypes.begin(), cpuTypes.end());
	_mainCpuType = cpuTypes[0];

	_labelManager.reset(new LabelManager(this));
	_memoryDumper.reset(new MemoryDumper(this));
	_disassembler.reset(new Disassembler(console, this));
	_disassemblySearch.reset(new DisassemblySearch(_disassembler.get(), _labelManager.get()));
	_memoryAccessCounter.reset(new MemoryAccessCounter(this));
	_scriptManager.reset(new ScriptManager(this));
	_traceLogSaver.reset(new TraceLogFileSaver());
	_cdlManager.reset(new CdlManager(this, _disassembler.get()));

	//Use cpuTypes for iteration (ordered), not _cpuTypes (order is important for coprocessors, etc.)
	for(CpuType type : cpuTypes) {
		unique_ptr<IDebugger> &debugger = _debuggers[(int)type].Debugger;
		switch(type) {
			case CpuType::Snes: debugger.reset(new SnesDebugger(this, CpuType::Snes)); break;
			case CpuType::Spc: debugger.reset(new SpcDebugger(this)); break;
			case CpuType::NecDsp: debugger.reset(new NecDspDebugger(this)); break;
			case CpuType::Sa1: debugger.reset(new SnesDebugger(this, CpuType::Sa1)); break;
			case CpuType::Gsu: debugger.reset(new GsuDebugger(this)); break;
			case CpuType::Cx4: debugger.reset(new Cx4Debugger(this)); break;
			case CpuType::St018: debugger.reset(new St018Debugger(this)); break;
			case CpuType::Gameboy: debugger.reset(new GbDebugger(this)); break;
			case CpuType::Nes: debugger.reset(new NesDebugger(this)); break;
			case CpuType::Pce: debugger.reset(new PceDebugger(this)); break;
			case CpuType::Sms: debugger.reset(new SmsDebugger(this)); break;
			case CpuType::Gba: debugger.reset(new GbaDebugger(this)); break;
			case CpuType::Ws: debugger.reset(new WsDebugger(this)); break;
			default: throw std::runtime_error("Unsupported CPU type");
		}

		_debuggers[(int)type].Evaluator.reset(new ExpressionEvaluator(this, _debuggers[(int)type].Debugger.get(), type));
	}

	for(CpuType type : _cpuTypes) {
		_debuggers[(int)type].Debugger->Init();
		_debuggers[(int)type].Debugger->ProcessConfigChange();
	}

	_breakRequestCount = 0;
	_suspendRequestCount = 0;

	_cdlManager->RefreshCodeCache();

	if(_emu->IsPaused()) {
		//Break on the current instruction if emulation was already paused
		Step(_mainCpuType, 1, StepType::Step, BreakSource::Pause);
	}

	_executionStopped = false;

#ifdef _DEBUG
	if(_mainCpuType == CpuType::Snes) {
		ExpressionEvaluator eval(this, _debuggers[(int)CpuType::Snes].Debugger.get(), CpuType::Snes);
		eval.RunTests();
	}
#endif
}

Debugger::~Debugger()
{
	Release();
}

void Debugger::Release()
{
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
		
		BaseEventManager* evtMgr = GetEventManager((CpuType)i);
		if(evtMgr) {
			//Call twice to clear both current and previous frame
			evtMgr->ClearFrameEvents();
			evtMgr->ClearFrameEvents();
		}
	}
}

template<CpuType type, typename DebuggerType>
DebuggerType* Debugger::GetDebugger()
{
	return (DebuggerType*)_debuggers[(int)type].Debugger.get();
}

IDebugger* Debugger::GetMainDebugger()
{
	return _debuggers[(int)_mainCpuType].Debugger.get();
}

template<CpuType type>
uint64_t Debugger::GetCpuCycleCount()
{
	switch(type) {
		case CpuType::Snes: return GetDebugger<type, SnesDebugger>()->GetCpuCycleCount();
		case CpuType::Spc: return GetDebugger<type, SpcDebugger>()->GetCpuCycleCount();
		case CpuType::NecDsp: return GetDebugger<type, NecDspDebugger>()->GetCpuCycleCount();
		case CpuType::Sa1: return GetDebugger<type, SnesDebugger>()->GetCpuCycleCount();
		case CpuType::Gsu: return GetDebugger<type, GsuDebugger>()->GetCpuCycleCount();
		case CpuType::Cx4: return GetDebugger<type, Cx4Debugger>()->GetCpuCycleCount();
		case CpuType::St018: return GetDebugger<type, St018Debugger>()->GetCpuCycleCount();
		case CpuType::Gameboy: return GetDebugger<type, GbDebugger>()->GetCpuCycleCount();
		case CpuType::Nes: return GetDebugger<type, NesDebugger>()->GetCpuCycleCount();
		case CpuType::Pce: return GetDebugger<type, PceDebugger>()->GetCpuCycleCount();
		case CpuType::Sms: return GetDebugger<type, SmsDebugger>()->GetCpuCycleCount();
		case CpuType::Gba: return GetDebugger<type, GbaDebugger>()->GetCpuCycleCount();
		case CpuType::Ws: return GetDebugger<type, WsDebugger>()->GetCpuCycleCount();
		default: return 0; break;
	}
}

bool Debugger::ProcessStepBack(IDebugger* debugger)
{
	if(debugger->CheckStepBack()) {
		//Step back target reached, break at the current instruction
		debugger->GetStepRequest()->Break(BreakSource::CpuStep);

		//Reset prev op code flag to prevent debugger code from incorrectly flagging
		//an instruction as the start of a function, etc. after loading the state
		debugger->ResetPrevOpCode();
		return false;
	} else {
		//While step back is running, don't process instructions
		return true;
	}
}

template<CpuType type>
void Debugger::ProcessInstruction()
{
	IDebugger* debugger = _debuggers[(int)type].Debugger.get();
	if(debugger->IsStepBack() && ProcessStepBack(debugger)) {
		debugger->AllowChangeProgramCounter = true; //set to true temporarily to allow debugger to pause on break requests when rewinding/step back is active
		SleepOnBreakRequest<type>();
		debugger->AllowChangeProgramCounter = false;
		return;
	}

	debugger->IgnoreBreakpoints = false;
	debugger->AllowChangeProgramCounter = true;

	switch(type) {
		case CpuType::Snes: GetDebugger<type, SnesDebugger>()->ProcessInstruction(); break;
		case CpuType::Spc: GetDebugger<type, SpcDebugger>()->ProcessInstruction(); break;
		case CpuType::NecDsp: GetDebugger<type, NecDspDebugger>()->ProcessInstruction(); break;
		case CpuType::Sa1: GetDebugger<type, SnesDebugger>()->ProcessInstruction(); break;
		case CpuType::Gsu: GetDebugger<type, GsuDebugger>()->ProcessInstruction(); break;
		case CpuType::Cx4: GetDebugger<type, Cx4Debugger>()->ProcessInstruction(); break;
		case CpuType::St018: GetDebugger<type, St018Debugger>()->ProcessInstruction(); break;
		case CpuType::Gameboy: GetDebugger<type, GbDebugger>()->ProcessInstruction(); break;
		case CpuType::Nes: GetDebugger<type, NesDebugger>()->ProcessInstruction(); break;
		case CpuType::Pce: GetDebugger<type, PceDebugger>()->ProcessInstruction(); break;
		case CpuType::Sms: GetDebugger<type, SmsDebugger>()->ProcessInstruction(); break;
		case CpuType::Gba: GetDebugger<type, GbaDebugger>()->ProcessInstruction(); break;
		case CpuType::Ws: GetDebugger<type, WsDebugger>()->ProcessInstruction(); break;
	}

	debugger->AllowChangeProgramCounter = false;
	
	if(_scriptManager->HasCpuMemoryCallbacks()) {
		MemoryOperationInfo memOp = debugger->InstructionProgress.LastMemOperation;
		AddressInfo relAddr = { (int32_t)memOp.Address, memOp.MemType };
		uint8_t value = (uint8_t)memOp.Value;
		_scriptManager->ProcessMemoryOperation(relAddr, value, MemoryOperationType::ExecOpCode, type, true);
	}
}

template<CpuType type, uint8_t accessWidth, MemoryAccessFlags flags, typename T>
void Debugger::ProcessMemoryRead(uint32_t addr, T& value, MemoryOperationType opType)
{
	if(_debuggers[(int)type].Debugger->IsStepBack()) {
		SleepOnBreakRequest<type>();
		return;
	}

	switch(type) {
		case CpuType::Snes: GetDebugger<CpuType::Snes, SnesDebugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::Spc: GetDebugger<CpuType::Spc, SpcDebugger>()->ProcessRead<flags>(addr, value, opType); break;
		case CpuType::NecDsp: GetDebugger<CpuType::NecDsp, NecDspDebugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::Sa1: GetDebugger<CpuType::Sa1, SnesDebugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::Gsu: GetDebugger<CpuType::Gsu, GsuDebugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::Cx4: GetDebugger<CpuType::Cx4, Cx4Debugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::St018: GetDebugger<CpuType::St018, St018Debugger>()->ProcessRead<accessWidth>(addr, value, opType); break;
		case CpuType::Gameboy: GetDebugger<CpuType::Gameboy, GbDebugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::Nes: GetDebugger<CpuType::Nes, NesDebugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::Pce: GetDebugger<CpuType::Pce, PceDebugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::Sms: GetDebugger<CpuType::Sms, SmsDebugger>()->ProcessRead(addr, value, opType); break;
		case CpuType::Gba: GetDebugger<CpuType::Gba, GbaDebugger>()->ProcessRead<accessWidth>(addr, value, opType); break;
		case CpuType::Ws:
			if constexpr(accessWidth <= 2) {
				GetDebugger<CpuType::Ws, WsDebugger>()->ProcessRead<accessWidth>(addr, value, opType);
			}
			break;
	}

	if(_scriptManager->HasCpuMemoryCallbacks()) {
		ProcessScripts<type>(addr, value, opType);
	}
}

template<CpuType type, uint8_t accessWidth, MemoryAccessFlags flags, typename T>
bool Debugger::ProcessMemoryWrite(uint32_t addr, T& value, MemoryOperationType opType)
{
	if(_debuggers[(int)type].Debugger->IsStepBack()) {
		SleepOnBreakRequest<type>();
		return !_debuggers[(int)type].Debugger->GetFrozenAddressManager().IsFrozenAddress(addr);
	}

	switch(type) {
		case CpuType::Snes: GetDebugger<CpuType::Snes, SnesDebugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::Spc: GetDebugger<CpuType::Spc, SpcDebugger>()->ProcessWrite<flags>(addr, value, opType); break;
		case CpuType::NecDsp: GetDebugger<CpuType::NecDsp, NecDspDebugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::Sa1: GetDebugger<CpuType::Sa1, SnesDebugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::Gsu: GetDebugger<CpuType::Gsu, GsuDebugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::Cx4: GetDebugger<CpuType::Cx4, Cx4Debugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::St018: GetDebugger<CpuType::St018, St018Debugger>()->ProcessWrite<accessWidth>(addr, value, opType); break;
		case CpuType::Gameboy: GetDebugger<CpuType::Gameboy, GbDebugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::Nes: GetDebugger<CpuType::Nes, NesDebugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::Pce: GetDebugger<CpuType::Pce, PceDebugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::Sms: GetDebugger<CpuType::Sms, SmsDebugger>()->ProcessWrite(addr, value, opType); break;
		case CpuType::Gba: GetDebugger<CpuType::Gba, GbaDebugger>()->ProcessWrite<accessWidth>(addr, value, opType); break;
		case CpuType::Ws:
			if constexpr(accessWidth <= 2) {
				GetDebugger<CpuType::Ws, WsDebugger>()->ProcessWrite<accessWidth>(addr, value, opType);
			}
			break;
	}
	
	if(_scriptManager->HasCpuMemoryCallbacks()) {
		ProcessScripts<type>(addr, value, opType);
	}
	
	return !_debuggers[(int)type].Debugger->GetFrozenAddressManager().IsFrozenAddress(addr);
}

template<CpuType cpuType, MemoryType memType, MemoryOperationType opType, typename T>
void Debugger::ProcessMemoryAccess(uint32_t addr, T& value)
{
	IDebugger* debugger = _debuggers[(int)cpuType].Debugger.get();

	constexpr int accessWidth = std::is_same<T, uint16_t>::value ? 2 : 1;

	if(debugger->IsStepBack()) {
		return;
	}

	AddressInfo addressInfo = { (int32_t)addr, memType };
	MemoryOperationInfo operation(addr, value, opType, memType);

	if constexpr(opType == MemoryOperationType::Write) {
		_memoryAccessCounter->ProcessMemoryWrite<accessWidth>(addressInfo, _emu->GetMasterClock());
	} else {
		_memoryAccessCounter->ProcessMemoryRead<accessWidth>(addressInfo, _emu->GetMasterClock());
	}

	switch(cpuType) {
		default: break;
		case CpuType::Sms: GetDebugger<CpuType::Sms, SmsDebugger>()->ProcessMemoryAccess<opType>(addr, value, memType); break;
		case CpuType::Ws: GetDebugger<CpuType::Ws, WsDebugger>()->ProcessMemoryAccess<opType, T>(addr, value, memType); break;
	}

	if(_scriptManager->HasCpuMemoryCallbacks()) {
		ProcessScripts<cpuType>(addr, value, memType, opType);
	}

	ProcessBreakConditions<accessWidth>(cpuType, *debugger->GetStepRequest(), debugger->GetBreakpointManager(), operation, addressInfo);
}

template<CpuType type>
void Debugger::ProcessIdleCycle()
{
	if(_debuggers[(int)type].Debugger->IsStepBack()) {
		SleepOnBreakRequest<type>();
		return;
	}

	_debuggers[(int)type].Debugger->InstructionProgress.LastMemOperation.Type = MemoryOperationType::Idle;

	switch(type) {
		case CpuType::Snes: GetDebugger<type, SnesDebugger>()->ProcessIdleCycle(); break;
		case CpuType::Sa1: GetDebugger<type, SnesDebugger>()->ProcessIdleCycle(); break;
	}
}

template<CpuType type>
void Debugger::ProcessHaltedCpu()
{
	IDebugger* dbg = _debuggers[(int)type].Debugger.get();
	if(dbg->IsStepBack() && ProcessStepBack(dbg)) {
		dbg->AllowChangeProgramCounter = true; //set to true temporarily to allow debugger to pause on break requests when rewinding/step back is active
		SleepOnBreakRequest<type>();
		dbg->AllowChangeProgramCounter = false;
		return;
	}

	//Set AllowChangeProgramCounter to allow SleepUntilResume to break properly
	dbg->AllowChangeProgramCounter = true;
	dbg->InstructionProgress.CurrentCycle = 0;
	
	//Process cpu step requests as if each call to ProcessHaltedCpu is an instruction
	StepRequest* req = dbg->GetStepRequest();
	req->ProcessCpuExec();
	if((int)req->BreakNeeded) {
		SleepUntilResume(type, req->GetBreakSource());
	} else {
		//Also check if a debugger break request is pending
		SleepOnBreakRequest<type>();
	}

	dbg->AllowChangeProgramCounter = false;
}

template<CpuType type>
void Debugger::SleepOnBreakRequest()
{
	if(_breakRequestCount) {
		SleepUntilResume(type, BreakSource::Unspecified);
	}
}

template<CpuType type, typename T>
void Debugger::ProcessPpuRead(uint16_t addr, T& value, MemoryType memoryType, MemoryOperationType opType)
{
	if(_debuggers[(int)type].Debugger->IsStepBack()) {
		return;
	}

	switch(type) {
		case CpuType::Snes: GetDebugger<type, SnesDebugger>()->ProcessPpuRead(addr, value, memoryType); break;
		case CpuType::Gameboy: GetDebugger<type, GbDebugger>()->ProcessPpuRead(addr, value, memoryType); break;
		case CpuType::Nes: GetDebugger<type, NesDebugger>()->ProcessPpuRead(addr, value, memoryType, opType); break;
		case CpuType::Pce: GetDebugger<type, PceDebugger>()->ProcessPpuRead(addr, value, memoryType); break;
		case CpuType::Sms: GetDebugger<type, SmsDebugger>()->ProcessPpuRead(addr, value, memoryType); break;
		default: throw std::runtime_error("Invalid cpu type");
	}

	if(_scriptManager->HasPpuMemoryCallbacks()) {
		ProcessScripts<type>(addr, value, memoryType, opType);
	}
}

template<CpuType type, typename T>
void Debugger::ProcessPpuWrite(uint16_t addr, T& value, MemoryType memoryType)
{
	if(_debuggers[(int)type].Debugger->IsStepBack()) {
		return;
	}

	switch(type) {
		case CpuType::Snes: GetDebugger<type, SnesDebugger>()->ProcessPpuWrite(addr, value, memoryType); break;
		case CpuType::Gameboy: GetDebugger<type, GbDebugger>()->ProcessPpuWrite(addr, value, memoryType); break;
		case CpuType::Nes: GetDebugger<type, NesDebugger>()->ProcessPpuWrite(addr, value, memoryType); break;
		case CpuType::Pce: GetDebugger<type, PceDebugger>()->ProcessPpuWrite(addr, value, memoryType); break;
		case CpuType::Sms: GetDebugger<type, SmsDebugger>()->ProcessPpuWrite(addr, value, memoryType); break;
		default: throw std::runtime_error("Invalid cpu type");
	}

	if(_scriptManager->HasPpuMemoryCallbacks()) {
		ProcessScripts<type>(addr, value, memoryType, MemoryOperationType::Write);
	}
}

template<CpuType type>
void Debugger::ProcessPpuCycle()
{
	if(_debuggers[(int)type].Debugger->IsStepBack()) {
		return;
	}

	switch(type) {
		case CpuType::Snes: GetDebugger<type, SnesDebugger>()->ProcessPpuCycle(); break;
		case CpuType::Gameboy: GetDebugger<type, GbDebugger>()->ProcessPpuCycle(); break;
		case CpuType::Nes: GetDebugger<type, NesDebugger>()->ProcessPpuCycle(); break;
		case CpuType::Pce: GetDebugger<type, PceDebugger>()->ProcessPpuCycle(); break;
		case CpuType::Sms: GetDebugger<type, SmsDebugger>()->ProcessPpuCycle(); break;
		case CpuType::Gba: GetDebugger<type, GbaDebugger>()->ProcessPpuCycle(); break;
		case CpuType::Ws: GetDebugger<type, WsDebugger>()->ProcessPpuCycle(); break;
		default: throw std::runtime_error("Invalid cpu type");
	}
}

void Debugger::SleepUntilResume(CpuType sourceCpu, BreakSource source, MemoryOperationInfo *operation, int breakpointId)
{
	if(_suspendRequestCount) {
		return;
	} else if(_breakRequestCount > 0 && (sourceCpu != _mainCpuType || !_debuggers[(int)sourceCpu].Debugger->AllowChangeProgramCounter)) {
		//When a break is requested by e.g a debugger call, load/save state, etc. always
		//break in-between 2 instructions of the main CPU, ensuring the state can be saved/loaded safely
		//If SleepUntilResume was called outside of ProcessInstruction, keep running
		return;
	} else if(IsBreakpointForbidden(source, sourceCpu, operation)) {
		ClearPendingBreakExceptions();
		return;
	}

	_executionStopped = true;
	
	bool notificationSent = false;
	if(source != BreakSource::Unspecified || _breakRequestCount == 0) {
		_emu->OnBeforePause(false);

		if(_settings->GetDebugConfig().SingleBreakpointPerInstruction) {
			_debuggers[(int)sourceCpu].Debugger->IgnoreBreakpoints = true;
		}

		if(_settings->GetDebugConfig().DrawPartialFrame) {
			_debuggers[(int)sourceCpu].Debugger->DrawPartialFrame();
		}

		//Only trigger code break event if the pause was caused by user action
		BreakEvent evt = {};
		evt.SourceCpu = sourceCpu;
		evt.BreakpointId = breakpointId;
		evt.Source = source;
		if(operation) {
			evt.Operation = *operation;
		}

		_waitForBreakResume = true;
		_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::CodeBreak, &evt);
		ProcessEvent(EventType::CodeBreak, sourceCpu);
		notificationSent = true;
		PlatformUtilities::EnableScreensaver();
	}

	while((_waitForBreakResume && !_suspendRequestCount) || _breakRequestCount) {
		std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(_breakRequestCount ? 1 : 10));
	}

	if(notificationSent) {
		PlatformUtilities::DisableScreensaver();
		_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::DebuggerResumed);
	}

	_executionStopped = false;
}

bool Debugger::IsBreakpointForbidden(BreakSource source, CpuType sourceCpu, MemoryOperationInfo* operation)
{
	if((source > BreakSource::InternalOperation || source == BreakSource::Breakpoint) && _breakRequestCount == 0) {
		BreakpointManager* bp = _debuggers[(int)sourceCpu].Debugger->GetBreakpointManager();
		uint32_t pc = GetProgramCounter(sourceCpu, true);
		AddressInfo relAddr = { (int32_t)pc, DebugUtilities::GetCpuMemoryType(sourceCpu) };
		AddressInfo absAddr = GetAbsoluteAddress(relAddr);
		return bp->IsForbidden(operation, relAddr, absAddr);
	}

	return false;
}

template<uint8_t accessWidth>
void Debugger::ProcessBreakConditions(CpuType sourceCpu, StepRequest& step, BreakpointManager* bpManager, MemoryOperationInfo& operation, AddressInfo& addressInfo)
{
	int breakpointId = bpManager->CheckBreakpoint<accessWidth>(operation, addressInfo, true);
	if(_breakRequestCount || _waitForBreakResume || ((int)step.BreakNeeded && (!_debuggers[(int)sourceCpu].Debugger->IgnoreBreakpoints || step.Type == StepType::CpuCycleStep))) {
		SleepUntilResume(sourceCpu, step.GetBreakSource());
	} else {
		if(breakpointId >= 0 && !_debuggers[(int)sourceCpu].Debugger->IgnoreBreakpoints) {
			SleepUntilResume(sourceCpu, BreakSource::Breakpoint, &operation, breakpointId);
		}
	}
}

template<uint8_t accessWidth>
void Debugger::ProcessPredictiveBreakpoint(CpuType sourceCpu, BreakpointManager* bpManager, MemoryOperationInfo& operation, AddressInfo& addressInfo)
{
	if(_debuggers[(int)sourceCpu].Debugger->IgnoreBreakpoints) {
		return;
	}

	int breakpointId = bpManager->CheckBreakpoint<accessWidth>(operation, addressInfo, false);
	if(breakpointId >= 0) {
		SleepUntilResume(sourceCpu, BreakSource::Breakpoint, &operation, breakpointId);
	}
}

template<CpuType type>
void Debugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	if(_debuggers[(int)type].Debugger->IsStepBack()) {
		return;
	}

	_debuggers[(int)type].Debugger->ProcessInterrupt(originalPc, currentPc, forNmi);
	ProcessEvent(forNmi ? EventType::Nmi : EventType::Irq, type);
}

void Debugger::InternalProcessInterrupt(CpuType cpuType, IDebugger& dbg, StepRequest& stepRequest, AddressInfo& src, uint32_t srcAddr, AddressInfo& dest, uint32_t destAddr, AddressInfo& ret, uint32_t retAddr, uint32_t retSp, bool forNmi)
{
	dbg.GetCallstackManager()->Push(src, srcAddr, dest, destAddr, ret, retAddr, retSp, forNmi ? StackFrameFlags::Nmi : StackFrameFlags::Irq);
	dbg.GetEventManager()->AddEvent(forNmi ? DebugEventType::Nmi : DebugEventType::Irq);
	stepRequest.ProcessNmiIrq(forNmi);
}

void Debugger::ProcessEvent(EventType type, std::optional<CpuType> cpuTypeOpt)
{
	CpuType evtCpuType = cpuTypeOpt.value_or(_mainCpuType);
	_scriptManager->ProcessEvent(type, evtCpuType);

	switch(type) {
		default: break;

		case EventType::InputPolled:
			_debuggers[(int)evtCpuType].Debugger->ProcessInputOverrides(_inputOverrides);
			break;

		case EventType::StartFrame: {
			_emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::EventViewerRefresh, (void*)evtCpuType);
			BaseEventManager* evtMgr = GetEventManager(evtCpuType);
			if(evtMgr) {
				evtMgr->ClearFrameEvents();
			}
			break;
		}

		case EventType::Reset:
			Reset();
			break;

		case EventType::StateLoaded:
			_memoryAccessCounter->ResetCounts();

			//Update the state for each cpu/debugger
			for(CpuType cpuType : _cpuTypes) {
				uint32_t pc = _debuggers[(int)cpuType].Debugger->GetProgramCounter(false);
				_debuggers[(int)cpuType].Debugger->SetProgramCounter(pc, true);

				CallstackManager* callstackManager = _debuggers[(int)cpuType].Debugger->GetCallstackManager();
				if(callstackManager) {
					callstackManager->Clear();
				}
			}
			break;
	}
}

template<CpuType type, typename T>
void Debugger::ProcessScripts(uint32_t addr, T& value, MemoryOperationType opType)
{
	MemoryOperationInfo memOp = GetDebugger<type, IDebugger>()->InstructionProgress.LastMemOperation;
	AddressInfo relAddr = { (int32_t)memOp.Address, memOp.MemType };
	_scriptManager->ProcessMemoryOperation(relAddr, value, opType, type, false);
}

template<CpuType type, typename T>
void Debugger::ProcessScripts(uint32_t addr, T& value, MemoryType memType, MemoryOperationType opType)
{
	AddressInfo relAddr = { (int32_t)addr, memType };
	_scriptManager->ProcessMemoryOperation(relAddr, value, opType, type, false);
}

void Debugger::ProcessConfigChange()
{
	for(int i = 0; i <= (int)DebugUtilities::GetLastCpuType(); i++) {
		if(_debuggers[i].Debugger) {
			_debuggers[i].Debugger->ProcessConfigChange();
		}
	}
}

void Debugger::GetTokenList(CpuType cpuType, char* tokenList)
{
	ExpressionEvaluator expEval(this, nullptr, cpuType);
	expEval.GetTokenList(tokenList);
}

int64_t Debugger::EvaluateExpression(string expression, CpuType cpuType, EvalResultType &resultType, bool useCache)
{
	MemoryOperationInfo operationInfo { 0, 0, MemoryOperationType::Read, MemoryType::None };
	AddressInfo addressInfo = { 0, MemoryType::None };
	if(useCache && _debuggers[(int)cpuType].Evaluator) {
		return _debuggers[(int)cpuType].Evaluator->Evaluate(expression, resultType, operationInfo, addressInfo);
	} else if(_debuggers[(int)cpuType].Debugger) {
		ExpressionEvaluator expEval(this, _debuggers[(int)cpuType].Debugger.get(), cpuType);
		return expEval.Evaluate(expression, resultType, operationInfo, addressInfo);
	}

	resultType = EvalResultType::Invalid;
	return 0;
}

void Debugger::Run()
{
	for(int i = 0; i <= (int)DebugUtilities::GetLastCpuType(); i++) {
		if(_debuggers[i].Debugger) {
			_debuggers[i].Debugger->ResetStepBackCache();
			_debuggers[i].Debugger->Run();
		}
	}
	_waitForBreakResume = false;
}

void Debugger::ClearPendingBreakExceptions()
{
	for(int i = 0; i <= (int)DebugUtilities::GetLastCpuType(); i++) {
		if(_debuggers[i].Debugger) {
			_debuggers[i].Debugger->GetStepRequest()->ClearException();
		}
	}
}

void Debugger::PauseOnNextFrame()
{
	//Use BreakSource::PpuStep to prevent "Run single frame" from triggering the "bring to front on pause" feature
	switch(_mainCpuType) {
		case CpuType::Snes: Step(CpuType::Snes, 240, StepType::SpecificScanline, BreakSource::PpuStep); break;
		case CpuType::Gameboy: Step(CpuType::Gameboy, 144, StepType::SpecificScanline, BreakSource::PpuStep); break;
		case CpuType::Nes: Step(CpuType::Nes, 241, StepType::SpecificScanline, BreakSource::PpuStep); break;
		case CpuType::Pce: Step(CpuType::Pce, 243, StepType::SpecificScanline, BreakSource::PpuStep); break;
		case CpuType::Sms: Step(CpuType::Sms, 240, StepType::SpecificScanline, BreakSource::PpuStep); break;
		case CpuType::Gba: Step(CpuType::Gba, 160, StepType::SpecificScanline, BreakSource::PpuStep); break;
		case CpuType::Ws: Step(CpuType::Ws, 145, StepType::SpecificScanline, BreakSource::PpuStep); break;
	}
}

void Debugger::Step(CpuType cpuType, int32_t stepCount, StepType type, BreakSource source)
{
	DebugBreakHelper helper(this);
	IDebugger* debugger = _debuggers[(int)cpuType].Debugger.get();

	if(debugger) {
		if(type != StepType::StepBack) {
			debugger->ResetStepBackCache();
		} else {
			debugger->StepBack(stepCount);
		}

		debugger->Step(stepCount, type);
		debugger->GetStepRequest()->SetBreakSource(source, false);
	}

	for(int i = 0; i <= (int)DebugUtilities::GetLastCpuType(); i++) {
		if(_debuggers[i].Debugger && _debuggers[i].Debugger.get() != debugger) {
			_debuggers[i].Debugger->ResetStepBackCache();
			_debuggers[i].Debugger->Run();
		}
	}

	_waitForBreakResume = false;
}

bool Debugger::IsPaused()
{
	return _waitForBreakResume;
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

void Debugger::ResetSuspendCounter()
{
	_suspendRequestCount = 0;
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

bool Debugger::IsDebugWindowOpened(CpuType cpuType)
{
	switch(cpuType) {
		case CpuType::Snes: return _settings->CheckDebuggerFlag(DebuggerFlags::SnesDebuggerEnabled);
		case CpuType::Spc: return _settings->CheckDebuggerFlag(DebuggerFlags::SpcDebuggerEnabled);
		case CpuType::NecDsp: return _settings->CheckDebuggerFlag(DebuggerFlags::NecDspDebuggerEnabled);
		case CpuType::Sa1: return _settings->CheckDebuggerFlag(DebuggerFlags::Sa1DebuggerEnabled);
		case CpuType::Gsu: return _settings->CheckDebuggerFlag(DebuggerFlags::GsuDebuggerEnabled);
		case CpuType::Cx4: return _settings->CheckDebuggerFlag(DebuggerFlags::Cx4DebuggerEnabled);
		case CpuType::St018: return _settings->CheckDebuggerFlag(DebuggerFlags::St018DebuggerEnabled);
		case CpuType::Gameboy: return _settings->CheckDebuggerFlag(DebuggerFlags::GbDebuggerEnabled);
		case CpuType::Nes: return _settings->CheckDebuggerFlag(DebuggerFlags::NesDebuggerEnabled);
		case CpuType::Pce: return _settings->CheckDebuggerFlag(DebuggerFlags::PceDebuggerEnabled);
		case CpuType::Sms: return _settings->CheckDebuggerFlag(DebuggerFlags::SmsDebuggerEnabled);
		case CpuType::Gba: return _settings->CheckDebuggerFlag(DebuggerFlags::GbaDebuggerEnabled);
		case CpuType::Ws: return _settings->CheckDebuggerFlag(DebuggerFlags::WsDebuggerEnabled);
	}

	return false;
}

bool Debugger::IsBreakOptionEnabled(BreakSource src)
{
	switch(src) {
		case BreakSource::GbDisableLcdOutsideVblank: return _settings->GetDebugConfig().GbBreakOnDisableLcdOutsideVblank;
		case BreakSource::GbInvalidVramAccess: return _settings->GetDebugConfig().GbBreakOnInvalidVramAccess;
		case BreakSource::GbInvalidOamAccess: return _settings->GetDebugConfig().GbBreakOnInvalidOamAccess;
		case BreakSource::NesBreakOnDecayedOamRead: return _settings->GetDebugConfig().NesBreakOnDecayedOamRead;
		case BreakSource::NesBreakOnPpu2000ScrollGlitch: return _settings->GetDebugConfig().NesBreakOnPpu2000ScrollGlitch;
		case BreakSource::NesBreakOnPpu2006ScrollGlitch: return _settings->GetDebugConfig().NesBreakOnPpu2006ScrollGlitch;
		case BreakSource::NesBusConflict: return _settings->GetDebugConfig().NesBreakOnBusConflict;
		case BreakSource::NesBreakOnCpuCrash: return _settings->GetDebugConfig().NesBreakOnCpuCrash;
		case BreakSource::NesBreakOnExtOutputMode: return _settings->GetDebugConfig().NesBreakOnExtOutputMode;
		case BreakSource::PceBreakOnInvalidVramAddress: return _settings->GetDebugConfig().PceBreakOnInvalidVramAddress;
		case BreakSource::GbaInvalidOpCode: return _settings->GetDebugConfig().GbaBreakOnInvalidOpCode;
		case BreakSource::GbaUnalignedMemoryAccess: return _settings->GetDebugConfig().GbaBreakOnUnalignedMemAccess;
	}
	return true;
}

void Debugger::BreakImmediately(CpuType sourceCpu, BreakSource source)
{
	if(_debuggers[(int)sourceCpu].Debugger->IsStepBack()) {
		return;
	}

	if(IsDebugWindowOpened(sourceCpu) && IsBreakOptionEnabled(source)) {
		SleepUntilResume(sourceCpu, source);
	}
}

void Debugger::GetCpuState(BaseState &dstState, CpuType cpuType)
{
	BaseState& srcState = GetCpuStateRef(cpuType);
	switch(cpuType) {
		case CpuType::Snes: memcpy(&dstState, &srcState, sizeof(SnesCpuState)); break;
		case CpuType::Spc: memcpy(&dstState, &srcState, sizeof(SpcState)); break;
		case CpuType::NecDsp: memcpy(&dstState, &srcState, sizeof(NecDspState)); break;
		case CpuType::Sa1: memcpy(&dstState, &srcState, sizeof(SnesCpuState)); break;
		case CpuType::Gsu: memcpy(&dstState, &srcState, sizeof(GsuState)); break;
		case CpuType::Cx4: memcpy(&dstState, &srcState, sizeof(Cx4State)); break;
		case CpuType::St018: memcpy(&dstState, &srcState, sizeof(ArmV3CpuState)); break;
		case CpuType::Gameboy: memcpy(&dstState, &srcState, sizeof(GbCpuState)); break;
		case CpuType::Nes: memcpy(&dstState, &srcState, sizeof(NesCpuState)); break;
		case CpuType::Pce: memcpy(&dstState, &srcState, sizeof(PceCpuState)); break;
		case CpuType::Sms: memcpy(&dstState, &srcState, sizeof(SmsCpuState)); break;
		case CpuType::Gba: memcpy(&dstState, &srcState, sizeof(GbaCpuState)); break;
		case CpuType::Ws: memcpy(&dstState, &srcState, sizeof(WsCpuState)); break;
	}
}

void Debugger::SetCpuState(BaseState& srcState, CpuType cpuType)
{
	DebugBreakHelper helper(this);
	BaseState& dstState = GetCpuStateRef(cpuType);
	switch(cpuType) {
		case CpuType::Snes: memcpy(&dstState, &srcState, sizeof(SnesCpuState)); break;
		case CpuType::Spc: memcpy(&dstState, &srcState, sizeof(SpcState)); break;
		case CpuType::NecDsp: memcpy(&dstState, &srcState, sizeof(NecDspState)); break;
		case CpuType::Sa1: memcpy(&dstState, &srcState, sizeof(SnesCpuState)); break;
		case CpuType::Gsu: memcpy(&dstState, &srcState, sizeof(GsuState)); break;
		case CpuType::Cx4: memcpy(&dstState, &srcState, sizeof(Cx4State)); break;
		case CpuType::St018: memcpy(&dstState, &srcState, sizeof(ArmV3CpuState)); break;
		case CpuType::Gameboy: memcpy(&dstState, &srcState, sizeof(GbCpuState)); break;
		case CpuType::Nes: memcpy(&dstState, &srcState, sizeof(NesCpuState)); break;
		case CpuType::Pce: memcpy(&dstState, &srcState, sizeof(PceCpuState)); break;
		case CpuType::Sms: memcpy(&dstState, &srcState, sizeof(SmsCpuState)); break;
		case CpuType::Gba: memcpy(&dstState, &srcState, sizeof(GbaCpuState)); break;
		case CpuType::Ws: memcpy(&dstState, &srcState, sizeof(WsCpuState)); break;
	}
}

BaseState& Debugger::GetCpuStateRef(CpuType cpuType)
{
	return _debuggers[(int)cpuType].Debugger->GetState();
}

void Debugger::GetPpuState(BaseState& state, CpuType cpuType)
{
	switch(cpuType) {
		case CpuType::Snes:
		case CpuType::Spc:
		case CpuType::NecDsp:
		case CpuType::Sa1:
		case CpuType::Gsu:
		case CpuType::Cx4:
		case CpuType::St018: {
			GetDebugger<CpuType::Snes, SnesDebugger>()->GetPpuState(state);
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

		case CpuType::Pce: {
			GetDebugger<CpuType::Pce, PceDebugger>()->GetPpuState(state);
			break;
		}

		case CpuType::Sms: {
			GetDebugger<CpuType::Sms, SmsDebugger>()->GetPpuState(state);
			break;
		}
		
		case CpuType::Gba: {
			GetDebugger<CpuType::Gba, GbaDebugger>()->GetPpuState(state);
			break;
		}
		
		case CpuType::Ws: {
			GetDebugger<CpuType::Ws, WsDebugger>()->GetPpuState(state);
			break;
		}
	}
}

void Debugger::SetPpuState(BaseState& state, CpuType cpuType)
{
	DebugBreakHelper helper(this);
	switch(cpuType) {
		case CpuType::Snes:
		case CpuType::Spc:
		case CpuType::NecDsp:
		case CpuType::Sa1:
		case CpuType::Gsu:
		case CpuType::Cx4:
		case CpuType::St018: {
			GetDebugger<CpuType::Snes, SnesDebugger>()->SetPpuState(state);
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

		case CpuType::Pce: {
			GetDebugger<CpuType::Pce, PceDebugger>()->SetPpuState(state);
			break;
		}

		case CpuType::Sms: {
			GetDebugger<CpuType::Sms, SmsDebugger>()->SetPpuState(state);
			break;
		}

		case CpuType::Gba: {
			GetDebugger<CpuType::Gba, GbaDebugger>()->SetPpuState(state);
			break;
		}

		case CpuType::Ws: {
			GetDebugger<CpuType::Ws, WsDebugger>()->SetPpuState(state);
			break;
		}
	}
}

void Debugger::GetConsoleState(BaseState& state, ConsoleType consoleType)
{
	_console->GetConsoleState(state, consoleType);
}

DebuggerFeatures Debugger::GetDebuggerFeatures(CpuType cpuType)
{
	if(_debuggers[(int)cpuType].Debugger) {
		return _debuggers[(int)cpuType].Debugger->GetSupportedFeatures();
	}
	return {};
}

void Debugger::SetProgramCounter(CpuType cpuType, uint32_t addr)
{
	if(_debuggers[(int)cpuType].Debugger->AllowChangeProgramCounter) {
		_debuggers[(int)cpuType].Debugger->SetProgramCounter(addr);
	}
}

uint32_t Debugger::GetProgramCounter(CpuType cpuType, bool getInstPc)
{
	return _debuggers[(int)cpuType].Debugger->GetProgramCounter(getInstPc);
}

uint8_t Debugger::GetCpuFlags(CpuType cpuType)
{
	return _debuggers[(int)cpuType].Debugger->GetCpuFlags();
}

CpuInstructionProgress Debugger::GetInstructionProgress(CpuType cpuType)
{
	CpuInstructionProgress progress = _debuggers[(int)cpuType].Debugger->InstructionProgress;
	progress.CurrentCycle = _debuggers[(int)cpuType].Debugger->GetCpuCycleCount();
	return progress;
}

AddressInfo Debugger::GetAbsoluteAddress(AddressInfo relAddress)
{
	return _console->GetAbsoluteAddress(relAddress);
}

AddressInfo Debugger::GetRelativeAddress(AddressInfo absAddress, CpuType cpuType)
{
	return _console->GetRelativeAddress(absAddress, cpuType);
}

bool Debugger::HasCpuType(CpuType cpuType)
{
	return _cpuTypes.find(cpuType) != _cpuTypes.end();
}

void Debugger::SetBreakpoints(Breakpoint breakpoints[], uint32_t length)
{
	DebugBreakHelper helper(this);
	for(int i = 0; i <= (int)DebugUtilities::GetLastCpuType(); i++) {
		if(_debuggers[i].Debugger) {
			_debuggers[i].Debugger->GetBreakpointManager()->SetBreakpoints(breakpoints, length);
		}
	}
}

void Debugger::SetInputOverrides(uint32_t index, DebugControllerState state)
{
	_inputOverrides[index] = state;
}

void Debugger::GetAvailableInputOverrides(uint8_t* availableIndexes)
{
	BaseControlManager* controlManager = _console->GetControlManager();
	for(int i = 0; i < 8; i++) {
		availableIndexes[i] = controlManager->GetControlDeviceByIndex(i) != nullptr;
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

bool Debugger::SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption)
{
	switch(_mainCpuType) {
		case CpuType::Snes:
			if(_debuggers[(int)CpuType::Gameboy].Debugger) {
				//SGB
				return GetDebugger<CpuType::Gameboy, GbDebugger>()->SaveRomToDisk(filename, saveAsIps, stripOption);
			} else {
				return GetDebugger<CpuType::Snes, SnesDebugger>()->SaveRomToDisk(filename, saveAsIps, stripOption);
			}

		case CpuType::Gameboy: return GetDebugger<CpuType::Gameboy, GbDebugger>()->SaveRomToDisk(filename, saveAsIps, stripOption);
		case CpuType::Nes: return GetDebugger<CpuType::Nes, NesDebugger>()->SaveRomToDisk(filename, saveAsIps, stripOption);
		case CpuType::Pce: return GetDebugger<CpuType::Pce, PceDebugger>()->SaveRomToDisk(filename, saveAsIps, stripOption);
		case CpuType::Sms: return GetDebugger<CpuType::Sms, SmsDebugger>()->SaveRomToDisk(filename, saveAsIps, stripOption);
		case CpuType::Gba: return GetDebugger<CpuType::Gba, GbaDebugger>()->SaveRomToDisk(filename, saveAsIps, stripOption);
		case CpuType::Ws: return GetDebugger<CpuType::Ws, WsDebugger>()->SaveRomToDisk(filename, saveAsIps, stripOption);
	}

	return false;
}

FrozenAddressManager* Debugger::GetFrozenAddressManager(CpuType cpuType)
{
	if(_debuggers[(int)cpuType].Debugger) {
		return &_debuggers[(int)cpuType].Debugger->GetFrozenAddressManager();
	}
	return nullptr;
}

ITraceLogger* Debugger::GetTraceLogger(CpuType cpuType)
{
	if(_debuggers[(int)cpuType].Debugger) {
		return _debuggers[(int)cpuType].Debugger->GetTraceLogger();
	}
	return nullptr;
}

void Debugger::ClearExecutionTrace()
{
	DebugBreakHelper helper(this);
	for(CpuType cpuType : _cpuTypes) {
		ITraceLogger* logger = GetTraceLogger(cpuType);
		logger->Clear();
	}
}

uint32_t Debugger::GetExecutionTrace(TraceRow output[], uint32_t startOffset, uint32_t maxLineCount)
{
	DebugBreakHelper helper(this);

	uint32_t offsetsByCpu[(int)DebugUtilities::GetLastCpuType() + 1] = {};

	uint32_t count = 0;
	int64_t lastRowId = ITraceLogger::NextRowId;
	while(count < maxLineCount) {
		bool added = false;
		for(CpuType cpuType : _cpuTypes) {
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
					if(logger->IsEnabled()) {
						if(output) {
							logger->GetExecutionTrace(output[count], offset);
						}
						count++;
					}
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

CallstackManager* Debugger::GetCallstackManager(CpuType cpuType)
{
	if(_debuggers[(int)cpuType].Debugger) {
		return _debuggers[(int)cpuType].Debugger->GetCallstackManager();
	}
	return nullptr;
}

IAssembler* Debugger::GetAssembler(CpuType cpuType)
{
	if(_debuggers[(int)cpuType].Debugger) {
		return _debuggers[(int)cpuType].Debugger->GetAssembler();
	}
	return nullptr;
}

template void Debugger::ProcessInstruction<CpuType::Snes>();
template void Debugger::ProcessInstruction<CpuType::Sa1>();
template void Debugger::ProcessInstruction<CpuType::Spc>();
template void Debugger::ProcessInstruction<CpuType::Gsu>();
template void Debugger::ProcessInstruction<CpuType::NecDsp>();
template void Debugger::ProcessInstruction<CpuType::Cx4>();
template void Debugger::ProcessInstruction<CpuType::St018>();
template void Debugger::ProcessInstruction<CpuType::Gameboy>();
template void Debugger::ProcessInstruction<CpuType::Nes>();
template void Debugger::ProcessInstruction<CpuType::Pce>();
template void Debugger::ProcessInstruction<CpuType::Sms>();
template void Debugger::ProcessInstruction<CpuType::Gba>();
template void Debugger::ProcessInstruction<CpuType::Ws>();

template void Debugger::ProcessMemoryRead<CpuType::Snes>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Sa1>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Spc>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Spc, 1, MemoryAccessFlags::DspAccess>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Gsu>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::NecDsp>(uint32_t addr, uint32_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::NecDsp>(uint32_t addr, uint16_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Cx4>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::St018, 1>(uint32_t addr, uint32_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::St018, 4>(uint32_t addr, uint32_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Gameboy>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Nes>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Pce>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Sms>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Gba, 1>(uint32_t addr, uint32_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Gba, 2>(uint32_t addr, uint32_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Gba, 4>(uint32_t addr, uint32_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Ws, 1>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template void Debugger::ProcessMemoryRead<CpuType::Ws, 2>(uint32_t addr, uint16_t& value, MemoryOperationType opType);

template bool Debugger::ProcessMemoryWrite<CpuType::Snes>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::Sa1>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::Spc>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::Spc, 1, MemoryAccessFlags::DspAccess>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::Gsu>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::NecDsp>(uint32_t addr, uint16_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::Cx4>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::St018, 1>(uint32_t addr, uint32_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::St018, 4>(uint32_t addr, uint32_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::Gameboy>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::Nes>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::Pce>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::Sms>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::Gba, 1>(uint32_t addr, uint32_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::Gba, 2>(uint32_t addr, uint32_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::Gba, 4>(uint32_t addr, uint32_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::Ws, 1>(uint32_t addr, uint8_t& value, MemoryOperationType opType);
template bool Debugger::ProcessMemoryWrite<CpuType::Ws, 2>(uint32_t addr, uint16_t& value, MemoryOperationType opType);

template void Debugger::ProcessMemoryAccess<CpuType::Pce, MemoryType::PceAdpcmRam, MemoryOperationType::Write>(uint32_t addr, uint8_t& value);
template void Debugger::ProcessMemoryAccess<CpuType::Pce, MemoryType::PceAdpcmRam, MemoryOperationType::Read>(uint32_t addr, uint8_t& value);
template void Debugger::ProcessMemoryAccess<CpuType::Pce, MemoryType::PceArcadeCardRam, MemoryOperationType::Write>(uint32_t addr, uint8_t& value);
template void Debugger::ProcessMemoryAccess<CpuType::Pce, MemoryType::PceArcadeCardRam, MemoryOperationType::Read>(uint32_t addr, uint8_t& value);
template void Debugger::ProcessMemoryAccess<CpuType::Sms, MemoryType::SmsPort, MemoryOperationType::Write>(uint32_t addr, uint8_t& value);
template void Debugger::ProcessMemoryAccess<CpuType::Sms, MemoryType::SmsPort, MemoryOperationType::Read>(uint32_t addr, uint8_t& value);

template void Debugger::ProcessMemoryAccess<CpuType::Ws, MemoryType::WsPort, MemoryOperationType::Write>(uint32_t addr, uint8_t& value);
template void Debugger::ProcessMemoryAccess<CpuType::Ws, MemoryType::WsPort, MemoryOperationType::Read>(uint32_t addr, uint8_t& value);
template void Debugger::ProcessMemoryAccess<CpuType::Ws, MemoryType::WsPort, MemoryOperationType::Write>(uint32_t addr, uint16_t& value);
template void Debugger::ProcessMemoryAccess<CpuType::Ws, MemoryType::WsPort, MemoryOperationType::Read>(uint32_t addr, uint16_t& value);
template void Debugger::ProcessMemoryAccess<CpuType::Ws, MemoryType::WsInternalEeprom, MemoryOperationType::Read>(uint32_t addr, uint16_t& value);
template void Debugger::ProcessMemoryAccess<CpuType::Ws, MemoryType::WsInternalEeprom, MemoryOperationType::Write>(uint32_t addr, uint16_t& value);
template void Debugger::ProcessMemoryAccess<CpuType::Ws, MemoryType::WsCartEeprom, MemoryOperationType::Read>(uint32_t addr, uint16_t& value);
template void Debugger::ProcessMemoryAccess<CpuType::Ws, MemoryType::WsCartEeprom, MemoryOperationType::Write>(uint32_t addr, uint16_t& value);

template void Debugger::ProcessIdleCycle<CpuType::Snes>();
template void Debugger::ProcessIdleCycle<CpuType::Sa1>();

template void Debugger::ProcessHaltedCpu<CpuType::Snes>();
template void Debugger::ProcessHaltedCpu<CpuType::Spc>();
template void Debugger::ProcessHaltedCpu<CpuType::Gameboy>();
template void Debugger::ProcessHaltedCpu<CpuType::Sms>();
template void Debugger::ProcessHaltedCpu<CpuType::Gba>();
template void Debugger::ProcessHaltedCpu<CpuType::Ws>();

template void Debugger::ProcessInterrupt<CpuType::Snes>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Debugger::ProcessInterrupt<CpuType::Sa1>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Debugger::ProcessInterrupt<CpuType::Gameboy>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Debugger::ProcessInterrupt<CpuType::Nes>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Debugger::ProcessInterrupt<CpuType::Pce>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Debugger::ProcessInterrupt<CpuType::Sms>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Debugger::ProcessInterrupt<CpuType::Gba>(uint32_t originalPc, uint32_t currentPc, bool forNmi);
template void Debugger::ProcessInterrupt<CpuType::Ws>(uint32_t originalPc, uint32_t currentPc, bool forNmi);

template void Debugger::ProcessPpuRead<CpuType::Snes>(uint16_t addr, uint8_t& value, MemoryType memoryType, MemoryOperationType opType);
template void Debugger::ProcessPpuRead<CpuType::Gameboy>(uint16_t addr, uint8_t& value, MemoryType memoryType, MemoryOperationType opType);
template void Debugger::ProcessPpuRead<CpuType::Nes>(uint16_t addr, uint8_t& value, MemoryType memoryType, MemoryOperationType opType);
template void Debugger::ProcessPpuRead<CpuType::Pce>(uint16_t addr, uint16_t& value, MemoryType memoryType, MemoryOperationType opType);
template void Debugger::ProcessPpuRead<CpuType::Pce>(uint16_t addr, uint8_t& value, MemoryType memoryType, MemoryOperationType opType);
template void Debugger::ProcessPpuRead<CpuType::Sms>(uint16_t addr, uint8_t& value, MemoryType memoryType, MemoryOperationType opType);

template void Debugger::ProcessPpuWrite<CpuType::Snes>(uint16_t addr, uint8_t& value, MemoryType memoryType);
template void Debugger::ProcessPpuWrite<CpuType::Gameboy>(uint16_t addr, uint8_t& value, MemoryType memoryType);
template void Debugger::ProcessPpuWrite<CpuType::Nes>(uint16_t addr, uint8_t& value, MemoryType memoryType);
template void Debugger::ProcessPpuWrite<CpuType::Pce>(uint16_t addr, uint16_t& value, MemoryType memoryType);
template void Debugger::ProcessPpuWrite<CpuType::Pce>(uint16_t addr, uint8_t& value, MemoryType memoryType);
template void Debugger::ProcessPpuWrite<CpuType::Sms>(uint16_t addr, uint8_t& value, MemoryType memoryType);

template void Debugger::ProcessPpuCycle<CpuType::Snes>();
template void Debugger::ProcessPpuCycle<CpuType::Gameboy>();
template void Debugger::ProcessPpuCycle<CpuType::Nes>();
template void Debugger::ProcessPpuCycle<CpuType::Pce>();
template void Debugger::ProcessPpuCycle<CpuType::Sms>();
template void Debugger::ProcessPpuCycle<CpuType::Gba>();
template void Debugger::ProcessPpuCycle<CpuType::Ws>();

template void Debugger::ProcessBreakConditions<1>(CpuType sourceCpu, StepRequest& step, BreakpointManager* bpManager, MemoryOperationInfo& operation, AddressInfo& addressInfo);
template void Debugger::ProcessBreakConditions<2>(CpuType sourceCpu, StepRequest& step, BreakpointManager* bpManager, MemoryOperationInfo& operation, AddressInfo& addressInfo);
template void Debugger::ProcessBreakConditions<4>(CpuType sourceCpu, StepRequest& step, BreakpointManager* bpManager, MemoryOperationInfo& operation, AddressInfo& addressInfo);

template void Debugger::ProcessPredictiveBreakpoint<1>(CpuType sourceCpu, BreakpointManager* bpManager, MemoryOperationInfo& operation, AddressInfo& addressInfo);
template void Debugger::ProcessPredictiveBreakpoint<2>(CpuType sourceCpu, BreakpointManager* bpManager, MemoryOperationInfo& operation, AddressInfo& addressInfo);
template void Debugger::ProcessPredictiveBreakpoint<4>(CpuType sourceCpu, BreakpointManager* bpManager, MemoryOperationInfo& operation, AddressInfo& addressInfo);
