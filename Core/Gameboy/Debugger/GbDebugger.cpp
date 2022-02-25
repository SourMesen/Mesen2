#include "stdafx.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbMemoryManager.h"
#include "Gameboy/Debugger/DummyGbCpu.h"
#include "Gameboy/Debugger/GbDebugger.h"
#include "Gameboy/Debugger/GameboyDisUtils.h"
#include "Gameboy/Debugger/GbEventManager.h"
#include "Gameboy/Debugger/GbTraceLogger.h"
#include "Gameboy/Debugger/GbPpuTools.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/BreakpointManager.h"
#include "Debugger/Debugger.h"
#include "Debugger/MemoryAccessCounter.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Debugger/MemoryDumper.h"
#include "Debugger/CodeDataLogger.h"
#include "Debugger/BaseEventManager.h"
#include "Utilities/Patches/IpsPatcher.h"
#include "Utilities/HexUtilities.h"
#include "Gameboy/Debugger/GbAssembler.h"
#include "Gameboy/GbPpu.h"
#include "Gameboy/GbCpu.h"
#include "SNES/SnesConsole.h"
#include "SNES/BaseCartridge.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"
#include "MemoryOperationType.h"

GbDebugger::GbDebugger(Debugger* debugger)
{
	_debugger = debugger;
	_emu = debugger->GetEmulator();

	_disassembler = debugger->GetDisassembler();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter();

	if(_emu->GetConsoleType() == ConsoleType::Snes) {
		_gameboy = ((SnesConsole*)debugger->GetConsole())->GetCartridge()->GetGameboy();
	} else {
		_gameboy = ((Gameboy*)debugger->GetConsole());
	}
	
	_cpu = _gameboy->GetCpu();
	_ppu = _gameboy->GetPpu();

	_settings = debugger->GetEmulator()->GetSettings();
	_codeDataLogger.reset(new CodeDataLogger(MemoryType::GbPrgRom, _gameboy->DebugGetMemorySize(MemoryType::GbPrgRom), CpuType::Gameboy));
	_traceLogger.reset(new GbTraceLogger(debugger, this, _ppu));
	_ppuTools.reset(new GbPpuTools(debugger, debugger->GetEmulator()));

	_eventManager.reset(new GbEventManager(debugger, _gameboy->GetCpu(), _ppu));
	_callstackManager.reset(new CallstackManager(debugger));
	_breakpointManager.reset(new BreakpointManager(debugger, this, CpuType::Gameboy, _eventManager.get()));
	_step.reset(new StepRequest());
	_assembler.reset(new GbAssembler(debugger->GetLabelManager()));
	
	_dummyCpu.reset(new DummyGbCpu());
	_dummyCpu->Init(_emu, _gameboy, _gameboy->GetMemoryManager());

	if(_gameboy->GetMasterClock() < 1000) {
		//Enable breaking on uninit reads when debugger is opened at power on
		_enableBreakOnUninitRead = true;
	}
}

GbDebugger::~GbDebugger()
{
}

void GbDebugger::Reset()
{
	_callstackManager.reset(new CallstackManager(_debugger));
	_prevOpCode = 0;
}

void GbDebugger::ProcessInstruction()
{
	GbCpuState& state = _cpu->GetState();
	uint16_t pc = state.PC;
	AddressInfo addressInfo = _gameboy->GetAbsoluteAddress(pc);
	uint8_t value = _gameboy->GetMemoryManager()->DebugRead(pc);
	MemoryOperationInfo operation(pc, value, MemoryOperationType::ExecOpCode, MemoryType::GameboyMemory);

	if(addressInfo.Address >= 0) {
		if(addressInfo.Type == MemoryType::GbPrgRom) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Code);
		}
		_disassembler->BuildCache(addressInfo, 0, CpuType::Gameboy);
	}

	if(GameboyDisUtils::IsJumpToSub(_prevOpCode) && pc != _prevProgramCounter + GameboyDisUtils::GetOpSize(_prevOpCode)) {
		//CALL and RST, and PC doesn't match the next instruction, so the call was (probably) done
		uint8_t opSize = DisassemblyInfo::GetOpSize(_prevOpCode, 0, CpuType::Gameboy);
		uint16_t returnPc = _prevProgramCounter + opSize;
		AddressInfo src = _gameboy->GetAbsoluteAddress(_prevProgramCounter);
		AddressInfo ret = _gameboy->GetAbsoluteAddress(returnPc);
		_callstackManager->Push(src, _prevProgramCounter, addressInfo, pc, ret, returnPc, StackFrameFlags::None);
	} else if(GameboyDisUtils::IsReturnInstruction(_prevOpCode)) {
		if(pc != _prevProgramCounter + GameboyDisUtils::GetOpSize(_prevOpCode)) {
			//RET used, and PC doesn't match the next instruction, so the ret was (probably) taken
			_callstackManager->Pop(addressInfo, pc);
		}

		if(_step->BreakAddress == (int32_t)pc) {
			//RET/RETI - if we're on the expected return address, break immediately (for step over/step out)
			_step->Break(BreakSource::CpuStep);
		}
	}

	if(_settings->CheckDebuggerFlag(DebuggerFlags::GbDebuggerEnabled)) {
		switch(value) {
			case 0x40:
				if(_settings->CheckDebuggerFlag(DebuggerFlags::GbBreakOnNopLoad)) {
					_step->Break(BreakSource::GbNopLoad);
				}
				break;

			case 0xD3: case 0xDB: case 0xDD: case 0xE3: case 0xE4: case 0xEB: case 0xEC: case 0xED: case 0xF4: case 0xFC: case 0xFD:
				if(_settings->CheckDebuggerFlag(DebuggerFlags::GbBreakOnInvalidOpCode)) {
					_step->Break(BreakSource::GbInvalidOpCode);
				}
				break;
		}
	}
	
	_prevOpCode = value;
	_prevProgramCounter = pc;

	_step->ProcessCpuExec();

	if(_step->StepCount != 0 && _breakpointManager->HasBreakpoints() && _settings->CheckDebuggerFlag(DebuggerFlags::UsePredictiveBreakpoints)) {
		_dummyCpu->SetDummyState(state);
		_dummyCpu->Exec();
		for(uint32_t i = 1; i < _dummyCpu->GetOperationCount(); i++) {
			MemoryOperationInfo memOp = _dummyCpu->GetOperationInfo(i);
			if(_breakpointManager->HasBreakpointForType(memOp.Type)) {
				AddressInfo absAddr = _gameboy->GetAbsoluteAddress(memOp.Address);
				_debugger->ProcessPredictiveBreakpoint(CpuType::Gameboy, _breakpointManager.get(), memOp, absAddr);
			}
		}
	}

	_debugger->ProcessBreakConditions(CpuType::Gameboy, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void GbDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _gameboy->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::GameboyMemory);

	if(type == MemoryOperationType::ExecOpCode) {
		if(_traceLogger->IsEnabled()) {
			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::Gameboy);
			_traceLogger->Log(_cpu->GetState(), disInfo, operation);
		}
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _emu->GetMasterClock());
	} else if(type == MemoryOperationType::ExecOperand) {
		if(addressInfo.Address >= 0 && addressInfo.Type == MemoryType::GbPrgRom) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Code);
		}

		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}

		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _emu->GetMasterClock());
		_debugger->ProcessBreakConditions(CpuType::Gameboy, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	} else {
		if(addressInfo.Address >= 0 && addressInfo.Type == MemoryType::GbPrgRom) {
			_codeDataLogger->SetFlags(addressInfo.Address, CdlFlags::Data);
		}

		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation);
		}

		if(addr < 0xFE00 || addr >= 0xFF80) {
			ReadResult result = _memoryAccessCounter->ProcessMemoryRead(addressInfo, _emu->GetMasterClock());
			if(result != ReadResult::Normal && _enableBreakOnUninitRead) {
				//Memory access was a read on an uninitialized memory address
				if(result == ReadResult::FirstUninitRead) {
					//Only warn the first time
					_debugger->Log("[GB] Uninitialized memory read: $" + HexUtilities::ToHex((uint16_t)addr));
				}
				if(_settings->CheckDebuggerFlag(DebuggerFlags::GbDebuggerEnabled) && _settings->CheckDebuggerFlag(DebuggerFlags::BreakOnUninitRead)) {
					_step->Break(BreakSource::BreakOnUninitMemoryRead);
				}
			}
		}

		if(addr == 0xFFFF || (addr >= 0xFE00 && addr < 0xFF80) || (addr >= 0x8000 && addr <= 0x9FFF)) {
			_eventManager->AddEvent(DebugEventType::Register, operation);
		}
		_debugger->ProcessBreakConditions(CpuType::Gameboy, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	}
}

void GbDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _gameboy->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::GameboyMemory);
	_debugger->ProcessBreakConditions(CpuType::Gameboy, *_step.get(), _breakpointManager.get(), operation, addressInfo);

	if(addressInfo.Type == MemoryType::GbWorkRam || addressInfo.Type == MemoryType::GbCartRam || addressInfo.Type == MemoryType::GbHighRam) {
		_disassembler->InvalidateCache(addressInfo, CpuType::Gameboy);
	}

	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation);
	}

	if(addr == 0xFFFF || (addr >= 0xFE00 && addr < 0xFF80) || (addr >= 0x8000 && addr <= 0x9FFF)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _emu->GetMasterClock());
}

void GbDebugger::Run()
{
	_step.reset(new StepRequest());
}

void GbDebugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step(type);

	switch(type) {
		case StepType::Step: step.StepCount = stepCount; break;
		case StepType::StepOut: step.BreakAddress = _callstackManager->GetReturnAddress(); break;
		case StepType::StepOver:
			if(GameboyDisUtils::IsJumpToSub(_prevOpCode)) {
				step.BreakAddress = _prevProgramCounter + DisassemblyInfo::GetOpSize(_prevOpCode, 0, CpuType::Gameboy);
			} else {
				//For any other instruction, step over is the same as step into
				step.StepCount = 1;
			}
			break;

		case StepType::PpuStep: step.PpuStepCount = stepCount; break;
		case StepType::PpuScanline: step.PpuStepCount = 456 * stepCount; break;
		case StepType::PpuFrame: step.PpuStepCount = 456*154; break;
		case StepType::SpecificScanline: step.BreakScanline = stepCount; break;
	}

	_step.reset(new StepRequest(step));
}

void GbDebugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	AddressInfo src = _gameboy->GetAbsoluteAddress(_prevProgramCounter);
	AddressInfo ret = _gameboy->GetAbsoluteAddress(originalPc);
	AddressInfo dest = _gameboy->GetAbsoluteAddress(currentPc);

	_debugger->InternalProcessInterrupt(
		CpuType::Gameboy, *this, *_step.get(), 
		src, _prevProgramCounter, dest, currentPc, ret, originalPc, forNmi
	);
}

void GbDebugger::ProcessPpuRead(uint16_t addr, uint8_t value, MemoryType memoryType)
{
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Read, memoryType);
	AddressInfo addressInfo { addr, memoryType };
	_debugger->ProcessBreakConditions(CpuType::Gameboy, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryRead(addressInfo, _gameboy->GetMasterClock());
}

void GbDebugger::ProcessPpuWrite(uint16_t addr, uint8_t value, MemoryType memoryType)
{
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Write, memoryType);
	AddressInfo addressInfo { addr, memoryType };
	_debugger->ProcessBreakConditions(CpuType::Gameboy, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _gameboy->GetMasterClock());
}

void GbDebugger::ProcessPpuCycle()
{
	if(_ppuTools->HasOpenedViewer()) {
		_ppuTools->UpdateViewers(_ppu->GetScanline(), _ppu->GetCycle());
	}

	if(_step->HasRequest) {
		if(_step->HasScanlineBreakRequest() && _ppu->GetCycle() == 0 && _ppu->GetScanline() == _step->BreakScanline) {
			_debugger->SleepUntilResume(CpuType::Gameboy, BreakSource::PpuStep);
		} else if(_step->PpuStepCount > 0) {
			_step->PpuStepCount--;
			if(_step->PpuStepCount == 0) {
				_debugger->SleepUntilResume(CpuType::Gameboy, BreakSource::PpuStep);
			}
		}
	}
}

DebuggerFeatures GbDebugger::GetSupportedFeatures()
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

void GbDebugger::SetProgramCounter(uint32_t addr)
{
	_cpu->GetState().PC = (uint16_t)addr;
	_prevOpCode = _gameboy->GetMemoryManager()->DebugRead((uint16_t)addr);
	_prevProgramCounter = (uint16_t)addr;
}

uint32_t GbDebugger::GetProgramCounter(bool getInstPc)
{
	return getInstPc ? _prevProgramCounter : _cpu->GetState().PC;
}

BaseEventManager* GbDebugger::GetEventManager()
{
	return _eventManager.get();
}

IAssembler* GbDebugger::GetAssembler()
{
	return _assembler.get();
}

CallstackManager* GbDebugger::GetCallstackManager()
{
	return _callstackManager.get();
}

CodeDataLogger* GbDebugger::GetCodeDataLogger()
{
	return _codeDataLogger.get();
}

BreakpointManager* GbDebugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

BaseState& GbDebugger::GetState()
{
	return _cpu->GetState();
}

void GbDebugger::GetPpuState(BaseState& state)
{
	(GbPpuState&)state = _ppu->GetStateRef();
}

void GbDebugger::SetPpuState(BaseState& srcState)
{
	GbPpuState& dstState = _ppu->GetStateRef();
	dstState = (GbPpuState&)srcState;
}

ITraceLogger* GbDebugger::GetTraceLogger()
{
	return _traceLogger.get();
}

PpuTools* GbDebugger::GetPpuTools()
{
	return _ppuTools.get();
}

void GbDebugger::SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption)
{
	vector<uint8_t> output;

	uint8_t* prgRom = _debugger->GetMemoryDumper()->GetMemoryBuffer(MemoryType::GbPrgRom);
	uint32_t prgRomSize = _debugger->GetMemoryDumper()->GetMemorySize(MemoryType::GbPrgRom);
	vector<uint8_t> rom = vector<uint8_t>(prgRom, prgRom + prgRomSize);

	if(saveAsIps) {
		vector<uint8_t> originalRom;
		_emu->GetRomInfo().RomFile.ReadFile(originalRom);

		output = IpsPatcher::CreatePatch(originalRom, rom);
	} else {
		if(stripOption != CdlStripOption::StripNone) {
			_codeDataLogger->StripData(rom.data(), stripOption);

			//Preserve rom header regardless of CDL file contents
			GameboyHeader header = _gameboy->GetHeader();
			memcpy(rom.data() + Gameboy::HeaderOffset, &header, sizeof(GameboyHeader));
		}
		output = rom;
	}

	ofstream file(filename, ios::out | ios::binary);
	if(file) {
		file.write((char*)output.data(), output.size());
		file.close();
	}
}
