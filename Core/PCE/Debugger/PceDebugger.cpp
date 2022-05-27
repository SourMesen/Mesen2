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
#include "PCE/PceConsole.h"
#include "PCE/PceCpu.h"
#include "PCE/PceVdc.h"
#include "PCE/PceVce.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/Debugger/PceDebugger.h"
#include "PCE/Debugger/PceTraceLogger.h"
#include "PCE/Debugger/PceVdcTools.h"
#include "PCE/Debugger/PceDisUtils.h"
#include "PCE/Debugger/DummyPceCpu.h"
#include "PCE/Debugger/PceEventManager.h"
#include "PCE/Debugger/PceAssembler.h"
#include "Utilities/HexUtilities.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/Patches/IpsPatcher.h"
#include "Shared/EmuSettings.h"
#include "Shared/SettingTypes.h"
#include "Shared/Emulator.h"
#include "MemoryOperationType.h"

PceDebugger::PceDebugger(Debugger* debugger)
{
	_debugger = debugger;
	_emu = debugger->GetEmulator();
	PceConsole* console = (PceConsole*)debugger->GetConsole();
	
	_console = console;
	_cpu = console->GetCpu();
	_vdc = console->GetVdc();
	_vce = console->GetVce();
	_memoryManager = console->GetMemoryManager();

	_traceLogger.reset(new PceTraceLogger(debugger, this, _vdc));
	_ppuTools.reset(new PceVdcTools(debugger, debugger->GetEmulator(), console));
	_disassembler = debugger->GetDisassembler();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter();
	_settings = debugger->GetEmulator()->GetSettings();

	_dummyCpu.reset(new DummyPceCpu(_emu, _memoryManager));

	_codeDataLogger.reset(new CodeDataLogger(MemoryType::PcePrgRom, _emu->GetMemory(MemoryType::PcePrgRom).Size, CpuType::Pce));

	_eventManager.reset(new PceEventManager(debugger, console));
	_callstackManager.reset(new CallstackManager(debugger));
	_breakpointManager.reset(new BreakpointManager(debugger, this, CpuType::Pce, _eventManager.get()));
	_step.reset(new StepRequest());
	_assembler.reset(new PceAssembler(_debugger->GetLabelManager()));

	if(_console->GetMasterClock() < 1000) {
		//Enable breaking on uninit reads when debugger is opened at power on
		_enableBreakOnUninitRead = true;
	}
}

void PceDebugger::Reset()
{
	_enableBreakOnUninitRead = true;
	_callstackManager.reset(new CallstackManager(_debugger));
	_prevOpCode = 0xFF;
}

void PceDebugger::ProcessInstruction()
{
	PceCpuState& state = _cpu->GetState();
	uint16_t addr = state.PC;
	uint8_t value = _memoryManager->DebugRead(addr);
	AddressInfo addressInfo = _memoryManager->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, MemoryOperationType::ExecOpCode, MemoryType::PceMemory);

	bool needDisassemble = _traceLogger->IsEnabled() || _settings->CheckDebuggerFlag(DebuggerFlags::PceDebuggerEnabled);
	if(addressInfo.Address >= 0) {
		if(addressInfo.Type == MemoryType::PcePrgRom) {
			uint8_t flags = CdlFlags::Code;
			if(PceDisUtils::IsJumpToSub(_prevOpCode)) {
				flags |= CdlFlags::SubEntryPoint;
			}
			_codeDataLogger->SetFlags(addressInfo.Address, flags);
		}
		if(needDisassemble) {
			_disassembler->BuildCache(addressInfo, 0, CpuType::Pce);
		}
	}

	uint32_t pc = state.PC;
	if(PceDisUtils::IsJumpToSub(_prevOpCode)) {
		//JSR
		uint8_t opSize = PceDisUtils::GetOpSize(_prevOpCode);
		uint32_t returnPc = (_prevProgramCounter + opSize) & 0xFFFF;
		AddressInfo srcAddress = _memoryManager->GetAbsoluteAddress(_prevProgramCounter);
		AddressInfo retAddress = _memoryManager->GetAbsoluteAddress(returnPc);
		_callstackManager->Push(srcAddress, _prevProgramCounter, addressInfo, pc, retAddress, returnPc, StackFrameFlags::None);
	} else if(PceDisUtils::IsReturnInstruction(_prevOpCode)) {
		//RTS, RTI
		_callstackManager->Pop(addressInfo, pc);
	}

	if(_step->BreakAddress == (int32_t)pc && PceDisUtils::IsReturnInstruction(_prevOpCode)) {
		//RTS/RTI found, if we're on the expected return address, break immediately (for step over/step out)
		_step->Break(BreakSource::CpuStep);
	}

	_prevOpCode = value;
	_prevProgramCounter = pc;

	_step->ProcessCpuExec();

	if(_settings->CheckDebuggerFlag(DebuggerFlags::PceDebuggerEnabled)) {
		if(value == 0x00 && _settings->CheckDebuggerFlag(DebuggerFlags::PceBreakOnBrk)) {
			_step->Break(BreakSource::BreakOnBrk);
		} else if(_settings->CheckDebuggerFlag(DebuggerFlags::PceBreakOnUnofficialOpCode) && PceDisUtils::IsOpUnofficial(value)) {
			_step->Break(BreakSource::BreakOnUnofficialOpCode);
		}
	}
	
	if(_step->StepCount != 0 && _breakpointManager->HasBreakpoints() && _settings->CheckDebuggerFlag(DebuggerFlags::UsePredictiveBreakpoints)) {
		_dummyCpu->SetDummyState(_cpu->GetState());
		_dummyCpu->Exec();
		for(uint32_t i = 1; i < _dummyCpu->GetOperationCount(); i++) {
			MemoryOperationInfo memOp = _dummyCpu->GetOperationInfo(i);
			if(_breakpointManager->HasBreakpointForType(memOp.Type)) {
				AddressInfo absAddr = _memoryManager->GetAbsoluteAddress(memOp.Address);
				_debugger->ProcessPredictiveBreakpoint(CpuType::Pce, _breakpointManager.get(), memOp, absAddr);
			}
		}
	}

	_debugger->ProcessBreakConditions(CpuType::Pce, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void PceDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _memoryManager->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::PceMemory);

	if(IsRegister(operation)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	if(type == MemoryOperationType::ExecOpCode) {
		if(_traceLogger->IsEnabled()) {
			PceCpuState& state = _cpu->GetState();
			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, state.PS, CpuType::Pce);
			_traceLogger->Log(state, disInfo, operation);
		}

		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _cpu->GetState().CycleCount);
	} else if(type == MemoryOperationType::ExecOperand) {
		if(addressInfo.Type == MemoryType::PcePrgRom && addressInfo.Address >= 0) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Code);
		}

		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}

		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _cpu->GetState().CycleCount);
		_debugger->ProcessBreakConditions(CpuType::Pce, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	} else {
		if(addressInfo.Type == MemoryType::PcePrgRom && addressInfo.Address >= 0) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Data);
		}
		
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}

		ReadResult result = _memoryAccessCounter->ProcessMemoryRead(addressInfo, _cpu->GetState().CycleCount);
		if(result != ReadResult::Normal && _enableBreakOnUninitRead) {
			//Memory access was a read on an uninitialized memory address
			if(result == ReadResult::FirstUninitRead) {
				//Only warn the first time
				_debugger->Log("[CPU] Uninitialized memory read: $" + HexUtilities::ToHex((uint16_t)addr));
			}
			if(_settings->CheckDebuggerFlag(DebuggerFlags::PceDebuggerEnabled) && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnUninitRead)) {
				_step->Break(BreakSource::BreakOnUninitMemoryRead);
			}
		}
		_debugger->ProcessBreakConditions(CpuType::Pce, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	}
}

void PceDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _memoryManager->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::PceMemory);
	if(addressInfo.Address >= 0 && (addressInfo.Type == MemoryType::PceWorkRam || addressInfo.Type == MemoryType::PceCardRam || addressInfo.Type == MemoryType::PceCdromRam)) {
		_disassembler->InvalidateCache(addressInfo, CpuType::Pce);
	}

	if(IsRegister(operation)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation);
	}

	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _cpu->GetState().CycleCount);
	
	_debugger->ProcessBreakConditions(CpuType::Pce, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void PceDebugger::Run()
{
	_step.reset(new StepRequest());
}

void PceDebugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step(type);
	switch(type) {
		case StepType::Step: step.StepCount = stepCount; break;
		case StepType::StepOut: step.BreakAddress = _callstackManager->GetReturnAddress(); break;
		case StepType::StepOver:
			if(PceDisUtils::IsJumpToSub(_prevOpCode)) {
				step.BreakAddress = (_prevProgramCounter + PceDisUtils::GetOpSize(_prevOpCode)) & 0xFFFF;
			} else {
				//For any other instruction, step over is the same as step into
				step.StepCount = 1;
			}
			break;

		case StepType::PpuStep: step.PpuStepCount = stepCount; break;
		case StepType::PpuScanline: step.PpuStepCount = PceConstants::ClockPerScanline * stepCount; break;
		case StepType::PpuFrame: step.PpuStepCount = PceConstants::ClockPerScanline * _vce->GetScanlineCount(); break;
		case StepType::SpecificScanline: step.BreakScanline = stepCount; break;
	}
	_step.reset(new StepRequest(step));
}

void PceDebugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	AddressInfo src = _memoryManager->GetAbsoluteAddress(_prevProgramCounter);
	AddressInfo ret = _memoryManager->GetAbsoluteAddress(originalPc);
	AddressInfo dest = _memoryManager->GetAbsoluteAddress(currentPc);
	
	_debugger->InternalProcessInterrupt(
		CpuType::Pce, *this, *_step.get(),
		src, _prevProgramCounter, dest, currentPc, ret, originalPc, forNmi
	);
}

void PceDebugger::ProcessPpuRead(uint16_t addr, uint8_t value, MemoryType memoryType)
{
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Read, memoryType);
	AddressInfo addressInfo { addr, memoryType };
	_debugger->ProcessBreakConditions(CpuType::Pce, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryRead(addressInfo, _console->GetMasterClock());
}

void PceDebugger::ProcessPpuWrite(uint16_t addr, uint8_t value, MemoryType memoryType)
{
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Write, memoryType);
	AddressInfo addressInfo { addr, memoryType };
	_debugger->ProcessBreakConditions(CpuType::Pce, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _console->GetMasterClock());
}

void PceDebugger::ProcessPpuCycle()
{
	if(_ppuTools->HasOpenedViewer()) {
		_ppuTools->UpdateViewers(_vdc->GetScanline(), _vdc->GetHClock());
	}

	if(_step->HasRequest) {
		if(_step->HasScanlineBreakRequest() && _vdc->GetHClock() == 0 && _vdc->GetScanline() == _step->BreakScanline) {
			_debugger->SleepUntilResume(CpuType::Pce, BreakSource::PpuStep);
		} else if(_step->PpuStepCount > 0) {
			_step->PpuStepCount -= 3;
			if(_step->PpuStepCount <= 0) {
				_step->PpuStepCount = 0;
				_debugger->SleepUntilResume(CpuType::Pce, BreakSource::PpuStep);
			}
		}
	}
}

bool PceDebugger::IsRegister(MemoryOperationInfo& op)
{
	uint8_t bank = _memoryManager->GetState().Mpr[op.Address >> 13];
	return bank == 0xFF;
}

DebuggerFeatures PceDebugger::GetSupportedFeatures()
{
	DebuggerFeatures features = {};
	features.RunToIrq = true;
	features.RunToNmi = false;
	features.StepOver = true;
	features.StepOut = true;
	features.CallStack = true;
	features.ChangeProgramCounter = AllowChangeProgramCounter;
	return features;
}

void PceDebugger::SetProgramCounter(uint32_t addr)
{
	_cpu->GetState().PC = (uint16_t)addr;
	_prevOpCode = _memoryManager->DebugRead(addr);
	_prevProgramCounter = (uint16_t)addr;
}

uint32_t PceDebugger::GetProgramCounter(bool getInstPc)
{
	return getInstPc ? _prevProgramCounter : _cpu->GetState().PC;
}

CallstackManager* PceDebugger::GetCallstackManager()
{
	return _callstackManager.get();
}

BreakpointManager* PceDebugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

IAssembler* PceDebugger::GetAssembler()
{
	return _assembler.get();
}

BaseEventManager* PceDebugger::GetEventManager()
{
	return _eventManager.get();
}

CodeDataLogger* PceDebugger::GetCodeDataLogger()
{
	return _codeDataLogger.get();
}

BaseState& PceDebugger::GetState()
{
	return _cpu->GetState();
}

void PceDebugger::GetPpuState(BaseState& state)
{
	(PceVideoState&)state = _console->GetVideoState();
}

void PceDebugger::SetPpuState(BaseState& state)
{
	//todo
	//_ppu->SetState((PceVdcState&)state);
}

ITraceLogger* PceDebugger::GetTraceLogger()
{
	return _traceLogger.get();
}

PpuTools* PceDebugger::GetPpuTools()
{
	return _ppuTools.get();
}

void PceDebugger::SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption)
{
	vector<uint8_t> output;

	uint8_t* prgRom = _debugger->GetMemoryDumper()->GetMemoryBuffer(MemoryType::PcePrgRom);
	uint32_t prgRomSize = _debugger->GetMemoryDumper()->GetMemorySize(MemoryType::PcePrgRom);
	vector<uint8_t> rom = vector<uint8_t>(prgRom, prgRom + prgRomSize);
	
	if(saveAsIps) {
		vector<uint8_t> originalRom;
		_emu->GetRomInfo().RomFile.ReadFile(originalRom);

		output = IpsPatcher::CreatePatch(originalRom, rom);
	} else {
		if(stripOption != CdlStripOption::StripNone) {
			_codeDataLogger->StripData(rom.data(), stripOption);
		}
		output = rom;
	}

	ofstream file(filename, ios::out | ios::binary);
	if(file) {
		file.write((char*)output.data(), output.size());
		file.close();
	}
}