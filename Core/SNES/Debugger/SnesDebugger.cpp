#include "stdafx.h"
#include "SNES/SnesCpu.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/BaseCartridge.h"
#include "SNES/SnesConsole.h"
#include "SNES/Spc.h"
#include "SNES/SnesPpu.h"
#include "SNES/MemoryMappings.h"
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
#include "Shared/SettingTypes.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/Patches/IpsPatcher.h"
#include "Utilities/CRC32.h"
#include "MemoryOperationType.h"

SnesDebugger::SnesDebugger(Debugger* debugger, CpuType cpuType)
{
	_cpuType = cpuType;

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
		_codeDataLogger->LoadCdlFile(_cdlFile, _settings->CheckDebuggerFlag(DebuggerFlags::AutoResetCdl));
	} else {
		_cdl = (SnesCodeDataLogger*)_debugger->GetCdlManager()->GetCodeDataLogger(MemoryType::SnesPrgRom);
	}

	_eventManager.reset(new SnesEventManager(debugger, _cpu, console->GetPpu(), _memoryManager, console->GetDmaController()));
	_callstackManager.reset(new CallstackManager(debugger));
	_breakpointManager.reset(new BreakpointManager(debugger, this, cpuType, _eventManager.get()));
	_step.reset(new StepRequest());
	_assembler.reset(new SnesAssembler(_debugger->GetLabelManager()));

	_dummyCpu.reset(new DummySnesCpu(_console, _cpuType));

	if(_console->GetMasterClock() < 1000) {
		//Enable breaking on uninit reads when debugger is opened at power on
		_enableBreakOnUninitRead = true;
	}
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
	_enableBreakOnUninitRead = true;
	_callstackManager.reset(new CallstackManager(_debugger));
	_prevOpCode = 0xFF;
}

void SnesDebugger::ProcessConfigChange()
{
	_debuggerEnabled = _settings->CheckDebuggerFlag(_cpuType == CpuType::Snes ? DebuggerFlags::SnesDebuggerEnabled : DebuggerFlags::Sa1DebuggerEnabled);
	_predictiveBreakpoints = _settings->CheckDebuggerFlag(DebuggerFlags::UsePredictiveBreakpoints);

	_runSpc = _spcTraceLogger->IsEnabled() || _settings->CheckDebuggerFlag(DebuggerFlags::SpcDebuggerEnabled);
	_runCoprocessors = (
		_dspTraceLogger && _dspTraceLogger->IsEnabled() || _settings->CheckDebuggerFlag(DebuggerFlags::NecDspDebuggerEnabled) ||
		_settings->CheckDebuggerFlag(DebuggerFlags::GbDebuggerEnabled)
	);
	_needCoprocessors = _runSpc || _runCoprocessors;
}

void SnesDebugger::ProcessInstruction()
{
	SnesCpuState& state = GetCpuState();
	uint32_t addr = (state.K << 16) | state.PC;
	AddressInfo addressInfo = _memoryMappings->GetAbsoluteAddress(addr);
	uint8_t value = _memoryManager->Peek(addr);
	MemoryOperationInfo operation(addr, value, MemoryOperationType::ExecOpCode, MemoryType::SnesMemory);

	if(addressInfo.Address >= 0) {
		uint8_t cpuFlags = state.PS & (ProcFlags::IndexMode8 | ProcFlags::MemoryMode8);
		if(addressInfo.Type == MemoryType::SnesPrgRom) {
			uint8_t flags = CdlFlags::Code | cpuFlags;
			if(SnesDisUtils::IsJumpToSub(_prevOpCode)) {
				flags |= CdlFlags::SubEntryPoint;
			}
			_cdl->SetSnesCode(addressInfo.Address, flags);
		}
		if(_traceLogger->IsEnabled() || _debuggerEnabled) {
			_disassembler->BuildCache(addressInfo, cpuFlags, _cpuType);
		}
	}

	if(SnesDisUtils::IsJumpToSub(_prevOpCode)) {
		//JSR, JSL
		uint8_t opSize = SnesDisUtils::GetOpSize(_prevOpCode, state.PS);
		uint32_t returnPc = (_prevProgramCounter & 0xFF0000) | (((_prevProgramCounter & 0xFFFF) + opSize) & 0xFFFF);
		AddressInfo srcAddress = _memoryMappings->GetAbsoluteAddress(_prevProgramCounter);
		AddressInfo retAddress = _memoryMappings->GetAbsoluteAddress(returnPc);
		_callstackManager->Push(srcAddress, _prevProgramCounter, addressInfo, addr, retAddress, returnPc, StackFrameFlags::None);
	} else if(SnesDisUtils::IsReturnInstruction(_prevOpCode)) {
		//RTS, RTL, RTI
		_callstackManager->Pop(addressInfo, addr);
	}

	if(_step->BreakAddress == (int32_t)addr && (SnesDisUtils::IsReturnInstruction(_prevOpCode) || _prevOpCode == 0x44 || _prevOpCode == 0x54)) {
		//RTS/RTL/RTI found, if we're on the expected return address, break immediately (for step over/step out)
		//Also used for MVN/MVP
		_step->Break(BreakSource::CpuStep);
	}

	_prevOpCode = value;
	_prevProgramCounter = addr;

	_step->ProcessCpuExec();

	if(_debuggerEnabled) {
		//Break on BRK/STP/WDM/COP
		switch(value) {
			case 0x00: if(_settings->CheckDebuggerFlag(DebuggerFlags::BreakOnBrk)) { _step->Break(BreakSource::BreakOnBrk); } break;
			case 0x02: if(_settings->CheckDebuggerFlag(DebuggerFlags::BreakOnCop)) { _step->Break(BreakSource::BreakOnCop); } break;
			case 0x42: if(_settings->CheckDebuggerFlag(DebuggerFlags::BreakOnWdm)) { _step->Break(BreakSource::BreakOnWdm); } break;
			case 0xDB: if(_settings->CheckDebuggerFlag(DebuggerFlags::BreakOnStp)) { _step->Break(BreakSource::BreakOnStp); } break;
		}
	}
	
	if(_step->StepCount != 0 && _breakpointManager->HasBreakpoints() && _predictiveBreakpoints) {
		_dummyCpu->SetDummyState(state);
		_dummyCpu->Exec();
		for(uint32_t i = 1; i < _dummyCpu->GetOperationCount(); i++) {
			MemoryOperationInfo memOp = _dummyCpu->GetOperationInfo(i);
			if(_breakpointManager->HasBreakpointForType(memOp.Type)) {
				AddressInfo absAddr = _memoryMappings->GetAbsoluteAddress(memOp.Address);
				_debugger->ProcessPredictiveBreakpoint(_cpuType, _breakpointManager.get(), memOp, absAddr);
			}
		}
	}

	_debugger->ProcessBreakConditions(_cpuType, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void SnesDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _memoryMappings->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::SnesMemory);
	SnesCpuState& state = GetCpuState();

	if(IsRegister(addr)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	if(type == MemoryOperationType::ExecOpCode) {
		if(_traceLogger->IsEnabled()) {
			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, state.PS, _cpuType);
			_traceLogger->Log(state, disInfo, operation);
		}
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
	} else if(type == MemoryOperationType::ExecOperand) {
		if(addressInfo.Type == MemoryType::SnesPrgRom && addressInfo.Address >= 0) {
			_cdl->SetSnesCode(addressInfo.Address, (state.PS & (SnesCdlFlags::IndexMode8 | SnesCdlFlags::MemoryMode8)));
		}
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetMasterClock());
		_debugger->ProcessBreakConditions(_cpuType, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	} else {
		if(addressInfo.Type == MemoryType::SnesPrgRom && addressInfo.Address >= 0) {
			_cdl->SetData(addressInfo.Address);
		}
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}

		ReadResult result = _memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetMasterClock());
		if(result != ReadResult::Normal && _enableBreakOnUninitRead) {
			//Memory access was a read on an uninitialized memory address
			if(result == ReadResult::FirstUninitRead) {
				//Only warn the first time
				_debugger->Log(string(_cpuType == CpuType::Sa1 ? "[SA1]" : "[CPU]") + " Uninitialized memory read: $" + HexUtilities::ToHex24(addr));
			}
			if(_debuggerEnabled && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnUninitRead)) {
				_step->Break(BreakSource::BreakOnUninitMemoryRead);
			}
		}
		_debugger->ProcessBreakConditions(_cpuType, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	}
}

void SnesDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _memoryMappings->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::SnesMemory);
	if(addressInfo.Address >= 0 && (addressInfo.Type == MemoryType::SnesWorkRam || addressInfo.Type == MemoryType::SnesSaveRam)) {
		_disassembler->InvalidateCache(addressInfo, _cpuType);
	}

	if(IsRegister(addr)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation);
	}

	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetMasterClock());

	_debugger->ProcessBreakConditions(_cpuType, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void SnesDebugger::Run()
{
	_step.reset(new StepRequest());
}

void SnesDebugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step(type);
	if((type == StepType::StepOver || type == StepType::StepOut || type == StepType::Step) && GetCpuState().StopState == SnesCpuStopState::Stopped) {
		//If STP was called, the CPU isn't running anymore - use the PPU to break execution instead (useful for test roms that end with STP)
		step.PpuStepCount = 1;
	} else {
		switch(type) {
			case StepType::Step: step.StepCount = stepCount; break;
			case StepType::StepOut: step.BreakAddress = _callstackManager->GetReturnAddress(); break;
			case StepType::StepOver:
				if(_prevOpCode == 0x20 || _prevOpCode == 0x22 || _prevOpCode == 0xFC || _prevOpCode == 0x00 || _prevOpCode == 0x02 || _prevOpCode == 0x44 || _prevOpCode == 0x54) {
					//JSR, JSL, BRK, COP, MVP, MVN
					step.BreakAddress = (_prevProgramCounter & 0xFF0000) | (((_prevProgramCounter & 0xFFFF) + SnesDisUtils::GetOpSize(_prevOpCode, 0)) & 0xFFFF);
				} else {
					//For any other instruction, step over is the same as step into
					step.StepCount = 1;
				}
				break;

			case StepType::PpuStep: step.PpuStepCount = stepCount; break;
			case StepType::PpuScanline: step.PpuStepCount = 341 * stepCount; break;
			case StepType::PpuFrame: step.PpuStepCount = 341 * (_ppu->GetVblankEndScanline() + 1); break;
			case StepType::SpecificScanline: step.BreakScanline = stepCount; break;
		}
	}
	_step.reset(new StepRequest(step));
}

void SnesDebugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	AddressInfo src = _memoryMappings->GetAbsoluteAddress(_prevProgramCounter);
	AddressInfo ret = _memoryMappings->GetAbsoluteAddress(originalPc);
	AddressInfo dest = _memoryMappings->GetAbsoluteAddress(currentPc);

	_debugger->InternalProcessInterrupt(
		_cpuType, *this, *_step.get(),
		src, _prevProgramCounter, dest, currentPc, ret, originalPc, forNmi
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
			_debugger->SleepUntilResume(CpuType::Snes, BreakSource::PpuStep);
		} else if(_step->PpuStepCount > 0) {
			_step->PpuStepCount--;
			if(_step->PpuStepCount == 0) {
				_debugger->SleepUntilResume(CpuType::Snes, BreakSource::PpuStep);
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
	features.CallStack = true;
	features.ChangeProgramCounter = AllowChangeProgramCounter;
	return features;
}

void SnesDebugger::SetProgramCounter(uint32_t addr)
{
	GetCpuState().PC = (uint16_t)addr;
	GetCpuState().K = (uint8_t)(addr >> 16);
	
	_prevOpCode = _memoryManager->Peek(addr);
	_prevProgramCounter = addr;
}

uint32_t SnesDebugger::GetProgramCounter(bool getInstPc)
{
	return getInstPc ? _prevProgramCounter : ((GetCpuState().K << 16) | GetCpuState().PC);
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

void SnesDebugger::SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption)
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
			_codeDataLogger->StripData(rom.data(), stripOption);

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
	}
}