#include "pch.h"
#include "SNES/SnesCpu.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/BaseCartridge.h"
#include "SNES/SnesConsole.h"
#include "SNES/Spc.h"
#include "SNES/SnesPpu.h"
#include "SNES/MemoryMappings.h"
#include "SNES/Input/SnesController.h"
#include "SNES/Debugger/DummySnesCpu.h"
#include "SNES/Debugger/SnesDisUtils.h"
#include "SNES/Debugger/SnesCodeDataLogger.h"
#include "SNES/Debugger/SnesAssembler.h"
#include "SNES/Debugger/SnesDebugger.h"
#include "SNES/Debugger/SnesEventManager.h"
#include "SNES/Debugger/TraceLogger/SnesCpuTraceLogger.h"
#include "SNES/Debugger/SnesPpuTools.h"
#include "Debugger/CdlManager.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/MemoryDumper.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Debugger/ScriptManager.h"
#include "Debugger/Debugger.h"
#include "Debugger/CodeDataLogger.h"
#include "Debugger/StepBackManager.h"
#include "Shared/SettingTypes.h"
#include "Shared/BaseControlManager.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/Patches/IpsPatcher.h"
#include "Utilities/CRC32.h"
#include "Shared/MemoryOperationType.h"

SnesDebugger::SnesDebugger(Debugger* debugger, CpuType cpuType) : IDebugger(debugger->GetEmulator())
{
	_cpuType = cpuType;
	_cpuMemType = _cpuType == CpuType::Snes ? MemoryType::SnesMemory : MemoryType::Sa1Memory;

	_debugger = debugger;
	_emu = debugger->GetEmulator();
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
	_traceLogger.reset(new SnesCpuTraceLogger(debugger, this, cpuType, _ppu, _memoryManager));
	_ppuTools.reset(new SnesPpuTools(debugger, debugger->GetEmulator()));
	
	if(_cpuType == CpuType::Snes) {
		_memoryMappings = _memoryManager->GetMemoryMappings();
	} else {
		_memoryMappings = _sa1->GetMemoryMappings();
	}

	if(cpuType == CpuType::Snes) {
		uint32_t crc32 = CRC32::GetCRC((uint8_t*)_emu->GetMemory(MemoryType::SnesPrgRom).Memory, _emu->GetMemory(MemoryType::SnesPrgRom).Size);
		_codeDataLogger.reset(new SnesCodeDataLogger(debugger, MemoryType::SnesPrgRom, console->GetCartridge()->DebugGetPrgRomSize(), CpuType::Snes, crc32));
		_cdl = _codeDataLogger.get();

		_cdlFile = _codeDataLogger->GetCdlFilePath(_console->GetCartridge()->GetGameboy() ? "SgbFirmware.cdl" : _emu->GetRomInfo().RomFile.GetFileName());
		_codeDataLogger->LoadCdlFile(_cdlFile, _settings->GetDebugConfig().AutoResetCdl);
	} else {
		_cdl = (SnesCodeDataLogger*)_debugger->GetCdlManager()->GetCodeDataLogger(MemoryType::SnesPrgRom);
	}
	
	_stepBackManager.reset(new StepBackManager(_emu, this));
	_eventManager.reset(new SnesEventManager(debugger, _cpu, console->GetPpu(), _memoryManager, console->GetDmaController()));
	_callstackManager.reset(new CallstackManager(debugger, this));
	_breakpointManager.reset(new BreakpointManager(debugger, this, cpuType, _eventManager.get()));
	_step.reset(new StepRequest());
	_assembler.reset(new SnesAssembler(_debugger->GetLabelManager()));

	_dummyCpu.reset(new DummySnesCpu(_console, _cpuType));
}

SnesDebugger::~SnesDebugger()
{
	if(_codeDataLogger) {
		_codeDataLogger->SaveCdlFile(_cdlFile);
	}
}

void SnesDebugger::Init()
{
	_spcTraceLogger = _debugger->GetTraceLogger(CpuType::Spc);
	_dspTraceLogger = _debugger->GetTraceLogger(CpuType::NecDsp);
}

void SnesDebugger::Reset()
{
	_callstackManager->Clear();
	ResetPrevOpCode();
}

void SnesDebugger::ProcessConfigChange()
{
	_debuggerEnabled = _settings->CheckDebuggerFlag(_cpuType == CpuType::Snes ? DebuggerFlags::SnesDebuggerEnabled : DebuggerFlags::Sa1DebuggerEnabled);
	_predictiveBreakpoints = _settings->GetDebugConfig().UsePredictiveBreakpoints;

	_runSpc = _spcTraceLogger->IsEnabled() || _settings->CheckDebuggerFlag(DebuggerFlags::SpcDebuggerEnabled);
	_runCoprocessors = (
		(_dspTraceLogger && _dspTraceLogger->IsEnabled()) || 
		_settings->CheckDebuggerFlag(DebuggerFlags::NecDspDebuggerEnabled) ||
		_settings->CheckDebuggerFlag(DebuggerFlags::GbDebuggerEnabled)
	);
	_needCoprocessors = _runSpc || _runCoprocessors;
}

uint64_t SnesDebugger::GetCpuCycleCount(bool forProfiler)
{
	if(forProfiler && _cpuType == CpuType::Snes) {
		return _memoryManager->GetMasterClock();
	} else {
		return GetCpuState().CycleCount;
	}
}

void SnesDebugger::ResetPrevOpCode()
{
	_prevOpCode = 0xFF;
}

void SnesDebugger::ProcessInstruction()
{
	SnesCpuState& state = GetCpuState();
	uint32_t pc = (state.K << 16) | state.PC;
	AddressInfo addressInfo = GetAbsoluteAddress(pc);
	uint8_t opCode = _memoryMappings->Peek(pc);
	MemoryOperationInfo operation(pc, opCode, MemoryOperationType::ExecOpCode, _cpuMemType);
	InstructionProgress.LastMemOperation = operation;
	InstructionProgress.StartCycle = state.CycleCount;

	if(addressInfo.Address >= 0) {
		uint8_t cpuFlags = state.PS & (ProcFlags::IndexMode8 | ProcFlags::MemoryMode8);
		if(addressInfo.Type == MemoryType::SnesPrgRom) {
			_cdl->SetCode(addressInfo.Address, SnesDisUtils::GetOpFlags(_prevOpCode, pc, _prevProgramCounter) | cpuFlags);
		}
		if(_traceLogger->IsEnabled() || _debuggerEnabled) {
			_disassembler->BuildCache(addressInfo, cpuFlags, _cpuType);
		}
	}

	ProcessCallStackUpdates(addressInfo, pc, state.PS, state.SP);

	if(_step->BreakAddress == (int32_t)pc && _step->BreakStackPointer == state.SP && (SnesDisUtils::IsReturnInstruction(_prevOpCode) || _prevOpCode == 0x44 || _prevOpCode == 0x54)) {
		//RTS/RTL/RTI found, if we're on the expected return address, break immediately (for step over/step out)
		//Also used for MVN/MVP
		_step->Break(BreakSource::CpuStep);
	}

	_prevOpCode = opCode;
	_prevProgramCounter = pc;
	_prevStackPointer = state.SP;

	_step->ProcessCpuExec();

	if(_debuggerEnabled) {
		//Break on BRK/STP/WDM/COP
		switch(opCode) {
			case 0x00: if(_settings->GetDebugConfig().SnesBreakOnBrk) { _step->Break(BreakSource::BreakOnBrk); } break;
			case 0x02: if(_settings->GetDebugConfig().SnesBreakOnCop) { _step->Break(BreakSource::BreakOnCop); } break;
			case 0x42: if(_settings->GetDebugConfig().SnesBreakOnWdm) { _step->Break(BreakSource::BreakOnWdm); } break;
			case 0xDB: if(_settings->GetDebugConfig().SnesBreakOnStp) { _step->Break(BreakSource::BreakOnStp); } break;
		}
	}
	
	if(_step->StepCount != 0 && _breakpointManager->HasBreakpoints() && _predictiveBreakpoints) {
		_dummyCpu->SetDummyState(state);
		_dummyCpu->Exec();
		for(uint32_t i = 1; i < _dummyCpu->GetOperationCount(); i++) {
			MemoryOperationInfo memOp = _dummyCpu->GetOperationInfo(i);
			if(_breakpointManager->HasBreakpointForType(memOp.Type)) {
				AddressInfo absAddr = GetAbsoluteAddress(memOp.Address);
				_debugger->ProcessPredictiveBreakpoint(_cpuType, _breakpointManager.get(), memOp, absAddr);
			}
		}
	}

	_debugger->ProcessBreakConditions(_cpuType, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void SnesDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, _cpuMemType);
	InstructionProgress.LastMemOperation = operation;
	SnesCpuState& state = GetCpuState();

	if(IsRegister(addr)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	if(type == MemoryOperationType::ExecOpCode) {
		if(_traceLogger->IsEnabled()) {
			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, state.PS, _cpuType);
			_traceLogger->Log(state, disInfo, operation, addressInfo);
		}
		
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
		if(_step->ProcessCpuCycle()) {
			_debugger->SleepUntilResume(_cpuType, BreakSource::CpuStep, &operation);
		}
	} else if(type == MemoryOperationType::ExecOperand) {
		if(addressInfo.Type == MemoryType::SnesPrgRom && addressInfo.Address >= 0) {
			_cdl->SetCode(addressInfo.Address, (state.PS & (SnesCdlFlags::IndexMode8 | SnesCdlFlags::MemoryMode8)));
		}
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation, addressInfo);
		}

		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
		_step->ProcessCpuCycle();
		_debugger->ProcessBreakConditions(_cpuType, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	} else {
		if(addressInfo.Type == MemoryType::SnesPrgRom && addressInfo.Address >= 0) {
			_cdl->SetData(addressInfo.Address);
		}
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation, addressInfo);
		}

		ReadResult result = _memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());
		if(result != ReadResult::Normal) {
			//Memory access was a read on an uninitialized memory address
			if(result == ReadResult::FirstUninitRead) {
				//Only warn the first time
				_debugger->Log(string(_cpuType == CpuType::Sa1 ? "[SA1]" : "[CPU]") + " Uninitialized memory read: $" + HexUtilities::ToHex24(addr));
			}
			if(_debuggerEnabled && _settings->GetDebugConfig().BreakOnUninitRead) {
				_step->Break(BreakSource::BreakOnUninitMemoryRead);
			}
		}
		
		if(type != MemoryOperationType::DmaRead) {
			_step->ProcessCpuCycle();
		}
		_debugger->ProcessBreakConditions(_cpuType, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	}
}

void SnesDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, _cpuMemType);
	InstructionProgress.LastMemOperation = operation;
	if(addressInfo.Address >= 0 && (addressInfo.Type == MemoryType::SnesWorkRam || addressInfo.Type == MemoryType::SnesSaveRam)) {
		_disassembler->InvalidateCache(addressInfo, _cpuType);
	}

	if(IsRegister(addr)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation, addressInfo);
	}

	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetMasterClock());

	if(type != MemoryOperationType::DmaWrite) {
		_step->ProcessCpuCycle();
	}
	_debugger->ProcessBreakConditions(_cpuType, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void SnesDebugger::ProcessIdleCycle()
{
	if(_step->ProcessCpuCycle()) {
		_debugger->SleepUntilResume(_cpuType, BreakSource::CpuStep);
	}
}

AddressInfo SnesDebugger::GetAbsoluteAddress(uint32_t addr)
{
	if(IsRegister(addr)) {
		return { (int32_t)(addr & 0xFFFF), MemoryType::SnesRegister };
	} else {
		return _memoryMappings->GetAbsoluteAddress(addr);
	}
}

void SnesDebugger::Run()
{
	_step.reset(new StepRequest());
}

void SnesDebugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step(type);
	switch(type) {
		case StepType::Step: step.StepCount = stepCount; break;
		case StepType::StepOut:
			step.BreakAddress = _callstackManager->GetReturnAddress();
			step.BreakStackPointer = _callstackManager->GetReturnStackPointer();
			break;

		case StepType::StepOver:
			if(_prevOpCode == 0x20 || _prevOpCode == 0x22 || _prevOpCode == 0xFC || _prevOpCode == 0x00 || _prevOpCode == 0x02 || _prevOpCode == 0x44 || _prevOpCode == 0x54) {
				//JSR, JSL, BRK, COP, MVP, MVN
				step.BreakAddress = (_prevProgramCounter & 0xFF0000) | (((_prevProgramCounter & 0xFFFF) + SnesDisUtils::GetOpSize(_prevOpCode, 0)) & 0xFFFF);
				step.BreakStackPointer = _prevStackPointer;
			} else {
				//For any other instruction, step over is the same as step into
				step.StepCount = 1;
			}
			break;

		case StepType::CpuCycleStep: step.CpuCycleStepCount = stepCount; break;
		case StepType::PpuStep: step.PpuStepCount = stepCount; break;
		case StepType::PpuScanline: step.PpuStepCount = 341 * stepCount; break;
		case StepType::PpuFrame: step.PpuStepCount = 341 * (_ppu->GetVblankEndScanline() + 1) * stepCount; break;
		case StepType::SpecificScanline: step.BreakScanline = stepCount; break;
	}
	_step.reset(new StepRequest(step));
}

StepBackConfig SnesDebugger::GetStepBackConfig()
{
	if(_cpuType == CpuType::Snes) {
		return {
			_memoryManager->GetMasterClock(),
			1364,
			1364u * (_ppu->GetVblankEndScanline() + 1)
		};
	} else {
		return IDebugger::GetStepBackConfig();
	}
}

void SnesDebugger::DrawPartialFrame()
{
	_ppu->DebugSendFrame();
}

void SnesDebugger::ProcessCallStackUpdates(AddressInfo& destAddr, uint32_t destPc, uint8_t cpuFlags, uint16_t sp)
{
	if(SnesDisUtils::IsJumpToSub(_prevOpCode)) {
		//JSR, JSL
		uint8_t opSize = SnesDisUtils::GetOpSize(_prevOpCode, cpuFlags);
		uint32_t returnPc = (_prevProgramCounter & 0xFF0000) | (((_prevProgramCounter & 0xFFFF) + opSize) & 0xFFFF);
		AddressInfo srcAddress = _memoryMappings->GetAbsoluteAddress(_prevProgramCounter);
		AddressInfo retAddress = _memoryMappings->GetAbsoluteAddress(returnPc);
		_callstackManager->Push(srcAddress, _prevProgramCounter, destAddr, destPc, retAddress, returnPc, _prevStackPointer, StackFrameFlags::None);
	} else if(SnesDisUtils::IsReturnInstruction(_prevOpCode)) {
		//RTS, RTL, RTI
		_callstackManager->Pop(destAddr, destPc, sp);
	}
}

void SnesDebugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	AddressInfo ret = _memoryMappings->GetAbsoluteAddress(originalPc);
	AddressInfo dest = _memoryMappings->GetAbsoluteAddress(currentPc);

	if(dest.Type == MemoryType::SnesPrgRom && dest.Address >= 0) {
		_cdl->SetCode(dest.Address, CdlFlags::SubEntryPoint);
	}

	//This assumes that the CPU is never running in emulation mode,
	//which is almost always true. This doesn't need to be perfect since
	//it only has minor impacts on the debugger (with step out/over)
	uint16_t originalSp = GetCpuState().SP + 4;
	_prevStackPointer = originalSp;

	//If a call/return occurred just before IRQ, it needs to be processed now
	ProcessCallStackUpdates(ret, originalPc, GetCpuState().PS, originalSp);
	ResetPrevOpCode();

	_debugger->InternalProcessInterrupt(
		_cpuType, *this, *_step.get(),
		ret, originalPc, dest, currentPc, ret, originalPc, originalSp, forNmi
	);
}

void SnesDebugger::ProcessPpuRead(uint16_t addr, uint8_t value, MemoryType memoryType)
{
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Read, memoryType);
	AddressInfo addressInfo { addr, memoryType };
	_debugger->ProcessBreakConditions(CpuType::Snes, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryRead(addressInfo, _console->GetMasterClock());
}

void SnesDebugger::ProcessPpuWrite(uint16_t addr, uint8_t value, MemoryType memoryType)
{
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Write, memoryType);
	AddressInfo addressInfo { addr, memoryType };
	_debugger->ProcessBreakConditions(CpuType::Snes, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _console->GetMasterClock());
}

void SnesDebugger::ProcessPpuCycle()
{
	if(_ppuTools->HasOpenedViewer()) {
		_ppuTools->UpdateViewers(_ppu->GetScanline(), _ppu->GetCycle());
	}

	if(_step->HasRequest) {
		if(_step->HasScanlineBreakRequest() && _ppu->GetScanline() == _step->BreakScanline && _memoryManager->GetHClock() == 0) {
			_debugger->SleepUntilResume(CpuType::Snes, _step->GetBreakSource());
		} else if(_step->PpuStepCount > 0) {
			_step->PpuStepCount--;
			if(_step->PpuStepCount == 0) {
				_debugger->SleepUntilResume(CpuType::Snes, _step->GetBreakSource());
			}
		}
	}

	//Catch up SPC/DSP as needed (if we're tracing or debugging those particular CPUs)
	if(_needCoprocessors) {
		if(_runSpc) {
			_spc->Run();
		}
		if(_runCoprocessors) {
			_cart->RunCoprocessors();
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

DebuggerFeatures SnesDebugger::GetSupportedFeatures()
{
	DebuggerFeatures features = {};
	features.RunToIrq = true;
	features.RunToNmi = true;
	features.StepOver = true;
	features.StepOut = true;
	features.StepBack = true;
	features.CallStack = true;
	features.ChangeProgramCounter = AllowChangeProgramCounter;
	features.CpuCycleStep = true;

	features.CpuVectors[0] = { "NMI", 0xFFEA };
	features.CpuVectors[1] = { "IRQ", 0xFFEE };
	features.CpuVectors[2] = { "Reset", 0xFFFC };
	features.CpuVectors[3] = { "BRK", 0xFFE6 };
	features.CpuVectors[4] = { "COP", 0xFFE4 };
	features.CpuVectors[5] = { "NMI (6502)", 0xFFFA };
	features.CpuVectors[6] = { "IRQ (6502)", 0xFFFE };
	features.CpuVectors[7] = { "COP (6502)", 0xFFF4 };
	features.CpuVectorCount = 8;

	return features;
}

void SnesDebugger::SetProgramCounter(uint32_t addr, bool updateDebuggerOnly)
{
	if(!updateDebuggerOnly) {
		GetCpuState().PC = (uint16_t)addr;
		GetCpuState().K = (uint8_t)(addr >> 16);
	}
	
	_prevOpCode = _memoryMappings->Peek(addr);
	_prevProgramCounter = addr;
	_prevStackPointer = GetCpuState().SP;
}

uint32_t SnesDebugger::GetProgramCounter(bool getInstPc)
{
	return getInstPc ? _prevProgramCounter : ((GetCpuState().K << 16) | GetCpuState().PC);
}

uint8_t SnesDebugger::GetCpuFlags()
{
	return GetCpuState().PS & (ProcFlags::IndexMode8 | ProcFlags::MemoryMode8);
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

bool SnesDebugger::SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption)
{
	vector<uint8_t> output;
	
	uint8_t* prgRom = _debugger->GetMemoryDumper()->GetMemoryBuffer(MemoryType::SnesPrgRom);
	uint32_t prgRomSize = _debugger->GetMemoryDumper()->GetMemorySize(MemoryType::SnesPrgRom);
	vector<uint8_t> rom = vector<uint8_t>(prgRom, prgRom + prgRomSize);

	if(saveAsIps) {
		vector<uint8_t> originalRom;
		_emu->GetRomInfo().RomFile.ReadFile(originalRom);

		output = IpsPatcher::CreatePatch(originalRom, rom);
	} else {
		if(stripOption != CdlStripOption::StripNone) {
			_cdl->StripData(rom.data(), stripOption);

			//Preserve rom header regardless of CDL file contents
			SnesCartInformation header = _cart->GetHeader();
			memcpy(rom.data() + _cart->GetHeaderOffset(), &header, sizeof(SnesCartInformation));
		}
		output = rom;
	}

	ofstream file(filename, ios::out | ios::binary);
	if(file) {
		file.write((char*)output.data(), output.size());
		file.close();
		return true;
	}
	return false;
}

void SnesDebugger::ProcessInputOverrides(DebugControllerState inputOverrides[8])
{
	BaseControlManager* controlManager = _console->GetControlManager();
	for(int i = 0; i < 8; i++) {
		shared_ptr<SnesController> controller = std::dynamic_pointer_cast<SnesController>(controlManager->GetControlDeviceByIndex(i));
		if(controller && inputOverrides[i].HasPressedButton()) {
			controller->SetBitValue(SnesController::Buttons::A, inputOverrides[i].A);
			controller->SetBitValue(SnesController::Buttons::B, inputOverrides[i].B);
			controller->SetBitValue(SnesController::Buttons::X, inputOverrides[i].X);
			controller->SetBitValue(SnesController::Buttons::Y, inputOverrides[i].Y);
			controller->SetBitValue(SnesController::Buttons::L, inputOverrides[i].L);
			controller->SetBitValue(SnesController::Buttons::R, inputOverrides[i].R);
			controller->SetBitValue(SnesController::Buttons::Select, inputOverrides[i].Select);
			controller->SetBitValue(SnesController::Buttons::Start, inputOverrides[i].Start);
			controller->SetBitValue(SnesController::Buttons::Up, inputOverrides[i].Up);
			controller->SetBitValue(SnesController::Buttons::Down, inputOverrides[i].Down);
			controller->SetBitValue(SnesController::Buttons::Left, inputOverrides[i].Left);
			controller->SetBitValue(SnesController::Buttons::Right, inputOverrides[i].Right);
		}
	}
	controlManager->RefreshHubState();
}
