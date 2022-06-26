#include "stdafx.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/CodeDataLogger.h"
#include "Debugger/ScriptManager.h"
#include "Debugger/Debugger.h"
#include "Debugger/MemoryDumper.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Debugger/CodeDataLogger.h"
#include "NES/NesHeader.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"
#include "NES/NesPpu.h"
#include "NES/BaseMapper.h"
#include "NES/NesMemoryManager.h"
#include "NES/Debugger/DummyNesCpu.h"
#include "NES/Debugger/NesCodeDataLogger.h"
#include "NES/Debugger/NesDebugger.h"
#include "NES/Debugger/NesAssembler.h"
#include "NES/Debugger/NesEventManager.h"
#include "NES/Debugger/NesTraceLogger.h"
#include "NES/Debugger/NesPpuTools.h"
#include "NES/Debugger/NesDisUtils.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/Patches/IpsPatcher.h"
#include "Utilities/CRC32.h"
#include "Shared/EmuSettings.h"
#include "Shared/SettingTypes.h"
#include "Shared/Emulator.h"
#include "MemoryOperationType.h"

NesDebugger::NesDebugger(Debugger* debugger)
{
	_debugger = debugger;
	_emu = debugger->GetEmulator();
	NesConsole* console = (NesConsole*)debugger->GetConsole();
	
	_console = console;
	_cpu = console->GetCpu();
	_ppu = console->GetPpu();
	_mapper = console->GetMapper();
	_memoryManager = console->GetMemoryManager();

	_traceLogger.reset(new NesTraceLogger(debugger, this, _ppu));
	_disassembler = debugger->GetDisassembler();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter();
	_settings = debugger->GetEmulator()->GetSettings();

	_dummyCpu.reset(new DummyNesCpu(_console));

	if(_emu->GetMemory(MemoryType::NesChrRom).Size > 0) {
		_chrRomCdl.reset(new CodeDataLogger(debugger, MemoryType::NesChrRom, _emu->GetMemory(MemoryType::NesChrRom).Size, CpuType::Nes, 0));
	}

	uint32_t crc32 = CRC32::GetCRC((uint8_t*)_emu->GetMemory(MemoryType::NesPrgRom).Memory, _emu->GetMemory(MemoryType::NesPrgRom).Size);
	_codeDataLogger.reset(new NesCodeDataLogger(debugger, MemoryType::NesPrgRom, _emu->GetMemory(MemoryType::NesPrgRom).Size, CpuType::Nes, crc32, _chrRomCdl.get()));

	string cdlFile;
	switch(_console->GetRomFormat()) {
		case RomFormat::Fds: cdlFile = "FdsFirmware.cdl"; break;
		case RomFormat::StudyBox: cdlFile = "StudyBoxFirmware.cdl"; break;
		default: cdlFile = _emu->GetRomInfo().RomFile.GetFileName(); break;
	}
	_cdlFile = _codeDataLogger->GetCdlFilePath(cdlFile);
	_codeDataLogger->LoadCdlFile(_cdlFile, _settings->CheckDebuggerFlag(DebuggerFlags::AutoResetCdl));

	_ppuTools.reset(new NesPpuTools(debugger, debugger->GetEmulator(), console));

	_eventManager.reset(new NesEventManager(debugger, console));
	_callstackManager.reset(new CallstackManager(debugger));
	_breakpointManager.reset(new BreakpointManager(debugger, this, CpuType::Nes, _eventManager.get()));
	_step.reset(new StepRequest());
	_assembler.reset(new NesAssembler(_debugger->GetLabelManager()));

	if(_console->GetMasterClock() < 1000) {
		//Enable breaking on uninit reads when debugger is opened at power on
		_enableBreakOnUninitRead = true;
	}
}

NesDebugger::~NesDebugger()
{
	_codeDataLogger->SaveCdlFile(_cdlFile);
}

void NesDebugger::Reset()
{
	_enableBreakOnUninitRead = true;
	_callstackManager.reset(new CallstackManager(_debugger));
	_prevOpCode = 0xFF;
}

void NesDebugger::ProcessInstruction()
{
	NesCpuState& state = _cpu->GetState();
	uint16_t pc = state.PC;
	uint8_t opCode = _memoryManager->DebugRead(pc);
	AddressInfo addressInfo = _mapper->GetAbsoluteAddress(pc);
	MemoryOperationInfo operation(pc, opCode, MemoryOperationType::ExecOpCode, MemoryType::NesMemory);

	bool needDisassemble = _traceLogger->IsEnabled() || _settings->CheckDebuggerFlag(DebuggerFlags::NesDebuggerEnabled);
	if(addressInfo.Address >= 0) {
		if(addressInfo.Type == MemoryType::NesPrgRom) {
			_codeDataLogger->SetCode(addressInfo.Address, NesDisUtils::GetOpFlags(_prevOpCode, pc, _prevProgramCounter));
		}
		if(needDisassemble) {
			_disassembler->BuildCache(addressInfo, 0, CpuType::Nes);
		}
	}

	if(NesDisUtils::IsJumpToSub(_prevOpCode)) {
		//JSR
		uint8_t opSize = NesDisUtils::GetOpSize(_prevOpCode);
		uint32_t returnPc = (_prevProgramCounter + opSize) & 0xFFFF;
		AddressInfo srcAddress = _mapper->GetAbsoluteAddress(_prevProgramCounter);
		AddressInfo retAddress = _mapper->GetAbsoluteAddress(returnPc);
		_callstackManager->Push(srcAddress, _prevProgramCounter, addressInfo, pc, retAddress, returnPc, StackFrameFlags::None);
	} else if(NesDisUtils::IsReturnInstruction(_prevOpCode)) {
		//RTS, RTI
		_callstackManager->Pop(addressInfo, pc);
	}

	if(_step->BreakAddress == (int32_t)pc && NesDisUtils::IsReturnInstruction(_prevOpCode)) {
		//RTS/RTI found, if we're on the expected return address, break immediately (for step over/step out)
		_step->Break(BreakSource::CpuStep);
	}

	_prevOpCode = opCode;
	_prevProgramCounter = pc;

	_step->ProcessCpuExec();

	if(_settings->CheckDebuggerFlag(DebuggerFlags::NesDebuggerEnabled)) {
		if(opCode == 0x00 && _settings->CheckDebuggerFlag(DebuggerFlags::NesBreakOnBrk)) {
			_step->Break(BreakSource::BreakOnBrk);
		} else if(_settings->CheckDebuggerFlag(DebuggerFlags::NesBreakOnUnofficialOpCode) && NesDisUtils::IsOpUnofficial(opCode)) {
			_step->Break(BreakSource::BreakOnUnofficialOpCode);
		}
	}

	if(_step->StepCount != 0 && _breakpointManager->HasBreakpoints() && _settings->CheckDebuggerFlag(DebuggerFlags::UsePredictiveBreakpoints)) {
		_dummyCpu->SetDummyState(_cpu);
		_dummyCpu->Exec();
		for(uint32_t i = 1; i < _dummyCpu->GetOperationCount(); i++) {
			MemoryOperationInfo memOp = _dummyCpu->GetOperationInfo(i);
			if(_breakpointManager->HasBreakpointForType(memOp.Type)) {
				AddressInfo absAddr = _mapper->GetAbsoluteAddress(memOp.Address);
				_debugger->ProcessPredictiveBreakpoint(CpuType::Nes, _breakpointManager.get(), memOp, absAddr);
			}
		}
	}

	_debugger->ProcessBreakConditions(CpuType::Nes, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void NesDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _mapper->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::NesMemory);

	if(IsRegister(operation)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	if(type == MemoryOperationType::ExecOpCode) {
		if(_traceLogger->IsEnabled()) {
			NesCpuState& state = _cpu->GetState();
			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, state.PS, CpuType::Nes);
			_traceLogger->Log(state, disInfo, operation);
		}

		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _cpu->GetCycleCount());
	} else if(type == MemoryOperationType::ExecOperand) {
		if(addressInfo.Type == MemoryType::NesPrgRom && addressInfo.Address >= 0) {
			_codeDataLogger->SetCode(addressInfo.Address);
		}

		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}

		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _cpu->GetCycleCount());
		_debugger->ProcessBreakConditions(CpuType::Nes, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	} else {
		if(operation.Type == MemoryOperationType::DmaRead) {
			_eventManager->AddEvent(DebugEventType::DmcDmaRead, operation);
		}

		if(addressInfo.Type == MemoryType::NesPrgRom && addressInfo.Address >= 0) {
			if(operation.Type == MemoryOperationType::DmaRead) {
				_codeDataLogger->SetData<NesCdlFlags::PcmData>(addressInfo.Address);
			} else {
				_codeDataLogger->SetData(addressInfo.Address);
			}
		}
		
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}

		ReadResult result = _memoryAccessCounter->ProcessMemoryRead(addressInfo, _cpu->GetCycleCount());
		if(result != ReadResult::Normal && _enableBreakOnUninitRead) {
			//Memory access was a read on an uninitialized memory address
			if(result == ReadResult::FirstUninitRead) {
				//Only warn the first time
				_debugger->Log("[CPU] Uninitialized memory read: $" + HexUtilities::ToHex((uint16_t)addr));
			}
			if(_settings->CheckDebuggerFlag(DebuggerFlags::NesDebuggerEnabled) && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnUninitRead)) {
				_step->Break(BreakSource::BreakOnUninitMemoryRead);
			}
		}
		_debugger->ProcessBreakConditions(CpuType::Nes, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	}
}

void NesDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _mapper->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::NesMemory);
	
	if(addressInfo.Address >= 0 && (addressInfo.Type == MemoryType::NesWorkRam || addressInfo.Type == MemoryType::NesSaveRam)) {
		_disassembler->InvalidateCache(addressInfo, CpuType::Nes);
	}

	if(IsRegister(operation)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation);
	}

	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _cpu->GetCycleCount());
	
	_debugger->ProcessBreakConditions(CpuType::Nes, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void NesDebugger::Run()
{
	_step.reset(new StepRequest());
}

void NesDebugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step(type);
	switch(type) {
		case StepType::Step: step.StepCount = stepCount; break;
		case StepType::StepOut: step.BreakAddress = _callstackManager->GetReturnAddress(); break;
		case StepType::StepOver:
			if(NesDisUtils::IsJumpToSub(_prevOpCode)) {
				//JSR, BRK
				step.BreakAddress = (_prevProgramCounter + NesDisUtils::GetOpSize(_prevOpCode)) & 0xFFFF;
			} else {
				//For any other instruction, step over is the same as step into
				step.StepCount = 1;
			}
			break;

		case StepType::PpuStep: step.PpuStepCount = stepCount; break;
		case StepType::PpuScanline: step.PpuStepCount = 341 * stepCount; break;
		case StepType::PpuFrame: step.PpuStepCount = 341 * _ppu->GetScanlineCount(); break;
		case StepType::SpecificScanline: step.BreakScanline = stepCount; break;
	}
	_step.reset(new StepRequest(step));
}

void NesDebugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	AddressInfo src = _mapper->GetAbsoluteAddress(_prevProgramCounter);
	AddressInfo ret = _mapper->GetAbsoluteAddress(originalPc);
	AddressInfo dest = _mapper->GetAbsoluteAddress(currentPc);

	if(dest.Type == MemoryType::NesPrgRom && dest.Address >= 0) {
		_codeDataLogger->SetCode(dest.Address, CdlFlags::SubEntryPoint);
	}

	_debugger->InternalProcessInterrupt(
		CpuType::Nes, *this, *_step.get(),
		src, _prevProgramCounter, dest, currentPc, ret, originalPc, forNmi
	);
}

void NesDebugger::ProcessPpuRead(uint16_t addr, uint8_t value, MemoryType memoryType, MemoryOperationType opType)
{
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Read, memoryType);
	AddressInfo addressInfo { addr, memoryType };
	if(DebugUtilities::IsRelativeMemory(memoryType)) {
		_mapper->GetPpuAbsoluteAddress(addr, addressInfo);
	}
	if(addressInfo.Type == MemoryType::NesChrRom && opType == MemoryOperationType::PpuRenderingRead) {
		_chrRomCdl->SetCode(addressInfo.Address);
	}
	_debugger->ProcessBreakConditions(CpuType::Nes, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryRead(addressInfo, _console->GetMasterClock());
}

void NesDebugger::ProcessPpuWrite(uint16_t addr, uint8_t value, MemoryType memoryType)
{
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Write, memoryType);
	AddressInfo addressInfo { addr, memoryType };
	if(DebugUtilities::IsRelativeMemory(memoryType)) {
		_mapper->GetPpuAbsoluteAddress(addr, addressInfo);
	}
	_debugger->ProcessBreakConditions(CpuType::Nes, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _console->GetMasterClock());
}

void NesDebugger::ProcessPpuCycle()
{
	if(_ppuTools->HasOpenedViewer()) {
		_ppuTools->UpdateViewers(_ppu->GetCurrentScanline(), _ppu->GetCurrentCycle());
	}

	if(_step->HasRequest) {
		if(_step->HasScanlineBreakRequest() && _ppu->GetCurrentCycle() == 0 && _ppu->GetCurrentScanline() == _step->BreakScanline) {
			_debugger->SleepUntilResume(CpuType::Nes, BreakSource::PpuStep);
		} else if(_step->PpuStepCount > 0) {
			_step->PpuStepCount--;
			if(_step->PpuStepCount == 0) {
				_debugger->SleepUntilResume(CpuType::Nes, BreakSource::PpuStep);
			}
		}
	}
}

bool NesDebugger::IsRegister(MemoryOperationInfo& op)
{
	if(op.Address >= 0x2000 && op.Address <= 0x3FFF) {
		return true;
	} else if(op.Address >= 0x4000 && op.Address <= 0x4015 || (op.Address == 0x4017 && op.Type == MemoryOperationType::Write)) {
		return true;
	} else if(op.Address == 0x4016 || (op.Address == 0x4017 && op.Type == MemoryOperationType::Read)) {
		return true;
	} else if(op.Address >= 0x4018 && ((op.Type == MemoryOperationType::Write && _mapper->IsWriteRegister(op.Address)) || (op.Type == MemoryOperationType::Read && _mapper->IsReadRegister(op.Address)))) {
		return true;
	}
	return false;
}

DebuggerFeatures NesDebugger::GetSupportedFeatures()
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

void NesDebugger::SetProgramCounter(uint32_t addr)
{
	_cpu->GetState().PC = (uint16_t)addr;
	_prevOpCode = _memoryManager->DebugRead(addr);
	_prevProgramCounter = (uint16_t)addr;
}

uint32_t NesDebugger::GetProgramCounter(bool getInstPc)
{
	return getInstPc ? _prevProgramCounter : _cpu->GetState().PC;
}

CallstackManager* NesDebugger::GetCallstackManager()
{
	return _callstackManager.get();
}

BreakpointManager* NesDebugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

IAssembler* NesDebugger::GetAssembler()
{
	return _assembler.get();
}

BaseEventManager* NesDebugger::GetEventManager()
{
	return _eventManager.get();
}

BaseState& NesDebugger::GetState()
{
	return _cpu->GetState();
}

void NesDebugger::GetPpuState(BaseState& state)
{
	_ppu->GetState((NesPpuState&)state);
}

void NesDebugger::SetPpuState(BaseState& state)
{
	_ppu->SetState((NesPpuState&)state);
}

ITraceLogger* NesDebugger::GetTraceLogger()
{
	return _traceLogger.get();
}

PpuTools* NesDebugger::GetPpuTools()
{
	return _ppuTools.get();
}

void NesDebugger::SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption)
{
	vector<uint8_t> output;

	uint8_t* prgRom = _debugger->GetMemoryDumper()->GetMemoryBuffer(MemoryType::NesPrgRom);
	uint32_t prgRomSize = _debugger->GetMemoryDumper()->GetMemorySize(MemoryType::NesPrgRom);
	vector<uint8_t> rom = vector<uint8_t>(prgRom, prgRom + prgRomSize);
	
	NesHeader header = _mapper->GetRomInfo().NesHeader;
	rom.insert(rom.begin(), (uint8_t*)&header, (uint8_t*)&header + sizeof(NesHeader));
	
	if(saveAsIps) {
		vector<uint8_t> originalRom;
		_emu->GetRomInfo().RomFile.ReadFile(originalRom);

		output = IpsPatcher::CreatePatch(originalRom, rom);
	} else {
		if(stripOption != CdlStripOption::StripNone) {
			_codeDataLogger->StripData(rom.data() + sizeof(NesHeader), stripOption);
		}
		output = rom;
	}

	ofstream file(filename, ios::out | ios::binary);
	if(file) {
		file.write((char*)output.data(), output.size());
		file.close();
	}
}