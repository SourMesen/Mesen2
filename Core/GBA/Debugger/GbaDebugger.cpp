#include "pch.h"
#include "GBA/GbaConsole.h"
#include "GBA/GbaMemoryManager.h"
#include "GBA/GbaPpu.h"
#include "GBA/GbaCpu.h"
#include "GBA/Input/GbaController.h"
#include "GBA/Debugger/DummyGbaCpu.h"
#include "GBA/Debugger/GbaDebugger.h"
#include "GBA/Debugger/GbaDisUtils.h"
#include "GBA/Debugger/GbaAssembler.h"
#include "GBA/Debugger/GbaEventManager.h"
#include "GBA/Debugger/GbaTraceLogger.h"
#include "GBA/Debugger/GbaCodeDataLogger.h"
#include "GBA/Debugger/GbaPpuTools.h"
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
#include "Debugger/StepBackManager.h"
#include "Utilities/Patches/IpsPatcher.h"
#include "Utilities/HexUtilities.h"
#include "Shared/EmuSettings.h"
#include "Shared/Emulator.h"
#include "Shared/BaseControlManager.h"
#include "Shared/MemoryOperationType.h"

GbaDebugger::GbaDebugger(Debugger* debugger) : IDebugger(debugger->GetEmulator())
{
	_debugger = debugger;
	_emu = debugger->GetEmulator();

	_disassembler = debugger->GetDisassembler();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter();

	_console = ((GbaConsole*)debugger->GetConsole());
	
	_cpu = _console->GetCpu();
	_ppu = _console->GetPpu();
	_memoryManager = _console->GetMemoryManager();

	_settings = debugger->GetEmulator()->GetSettings();
	
	_codeDataLogger.reset(new GbaCodeDataLogger(debugger, MemoryType::GbaPrgRom, _emu->GetMemory(MemoryType::GbaPrgRom).Size, CpuType::Gba, _emu->GetCrc32()));
	_cdlFile = _codeDataLogger->GetCdlFilePath(_emu->GetRomInfo().RomFile.GetFileName());
	_codeDataLogger->LoadCdlFile(_cdlFile, _settings->GetDebugConfig().AutoResetCdl);

	_traceLogger.reset(new GbaTraceLogger(debugger, this, _ppu));
	_ppuTools.reset(new GbaPpuTools(debugger, debugger->GetEmulator()));

	_stepBackManager.reset(new StepBackManager(_emu, this));
	_eventManager.reset(new GbaEventManager(debugger, _console->GetCpu(), _ppu, _memoryManager, _console->GetDmaController()));
	_callstackManager.reset(new CallstackManager(debugger, this));
	_breakpointManager.reset(new BreakpointManager(debugger, this, CpuType::Gba, _eventManager.get()));
	_step.reset(new StepRequest());
	_assembler.reset(new GbaAssembler(debugger->GetLabelManager()));
	
	_dummyCpu.reset(new DummyGbaCpu());
	_dummyCpu->Init(_emu, _memoryManager, nullptr);
}

GbaDebugger::~GbaDebugger()
{
	_codeDataLogger->SaveCdlFile(_cdlFile);
}

void GbaDebugger::Reset()
{
	_callstackManager->Clear();
	ResetPrevOpCode();
}

void GbaDebugger::ProcessInstruction()
{
	if(GbaDisUtils::IsThumbMode(_cpu->GetState().CPSR.ToInt32())) {
		ProcessInstruction<2>();
	} else {
		ProcessInstruction<4>();
	}
}

template<uint8_t accessWidth>
void GbaDebugger::ProcessInstruction()
{
	GbaCpuState& state = _cpu->GetState();
	uint32_t pc = state.Pipeline.Execute.Address;
	uint32_t opCode = state.Pipeline.Execute.OpCode;
	
	uint8_t flags = accessWidth == 2 ? GbaCdlFlags::Thumb : 0; //thumb flag only

	AddressInfo addressInfo = _memoryManager->GetAbsoluteAddress(pc);
	MemoryOperationInfo operation(pc, opCode, MemoryOperationType::ExecOpCode, MemoryType::GbaMemory);
	InstructionProgress.LastMemOperation = operation;
	InstructionProgress.StartCycle = _memoryManager->GetMasterClock();

	if(addressInfo.Type != MemoryType::None) {
		if(addressInfo.Type == MemoryType::GbaPrgRom) {
			_codeDataLogger->SetCode<accessWidth>(addressInfo.Address, GbaDisUtils::GetOpFlags(_prevOpCode, _prevFlags, pc, _prevProgramCounter) | flags);
		}
		_disassembler->BuildCache(addressInfo, flags, CpuType::Gba);
	}

	ProcessCallStackUpdates(addressInfo, pc);

	if(_settings->CheckDebuggerFlag(DebuggerFlags::GbaDebuggerEnabled)) {
		if(((accessWidth == 2 && opCode == 0x46DB) || (accessWidth == 4 && opCode == 0xE1A0B00B)) && _settings->GetDebugConfig().GbaBreakOnNopLoad) {
			//Break on MOV R11, R11
			_step->Break(BreakSource::GbaNopLoad);
		}
	}
	
	_prevFlags = flags;
	_prevOpCode = opCode;
	_prevProgramCounter = pc;

	_step->ProcessCpuExec();

	if(_step->StepCount != 0 && _breakpointManager->HasBreakpoints() && _settings->GetDebugConfig().UsePredictiveBreakpoints) {
		_dummyCpu->SetDummyState(state);
		_dummyCpu->Exec<false, false>();
		for(uint32_t i = 1; i < _dummyCpu->GetOperationCount(); i++) {
			MemoryOperationInfo memOp = _dummyCpu->GetOperationInfo(i);
			if(_breakpointManager->HasBreakpointForType(memOp.Type)) {
				AddressInfo absAddr = _memoryManager->GetAbsoluteAddress(memOp.Address);
				switch(_dummyCpu->GetOperationMode(i) & (GbaAccessMode::Byte | GbaAccessMode::HalfWord | GbaAccessMode::Word)) {
					case GbaAccessMode::Byte: _debugger->ProcessPredictiveBreakpoint<1>(CpuType::Gba, _breakpointManager.get(), memOp, absAddr); break;
					case GbaAccessMode::HalfWord: _debugger->ProcessPredictiveBreakpoint<2>(CpuType::Gba, _breakpointManager.get(), memOp, absAddr); break;
					case GbaAccessMode::Word: _debugger->ProcessPredictiveBreakpoint<4>(CpuType::Gba, _breakpointManager.get(), memOp, absAddr); break;
				}
			}
		}
	}

	_debugger->ProcessBreakConditions<accessWidth>(CpuType::Gba, *_step.get(), _breakpointManager.get(), operation, addressInfo);

	if(_traceLogger->IsEnabled()) {
		DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, pc, _prevFlags, CpuType::Gba);
		_traceLogger->Log(state, disInfo, operation, addressInfo);
	}
}

template<uint8_t accessWidth>
void GbaDebugger::ProcessRead(uint32_t addr, uint32_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _memoryManager->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::GbaMemory);
	InstructionProgress.LastMemOperation = operation;

	if(type == MemoryOperationType::ExecOpCode) {
		_memoryAccessCounter->ProcessMemoryExec<accessWidth>(addressInfo, _console->GetMasterClock());
	} else {
		if(addressInfo.Address >= 0) {
			if(addressInfo.Type == MemoryType::GbaPrgRom) {
				_codeDataLogger->SetData<0, accessWidth>(addressInfo.Address);
			}

			ReadResult result = _memoryAccessCounter->ProcessMemoryRead<accessWidth>(addressInfo, _console->GetMasterClock());
			if(result != ReadResult::Normal) {
				//Memory access was a read on an uninitialized memory address
				if((int)result & (int)ReadResult::FirstUninitRead) {
					//Only warn the first time
					_debugger->Log("[GBA] Uninitialized memory read: $" + HexUtilities::ToHex(addr));
				}
				if(_settings->CheckDebuggerFlag(DebuggerFlags::GbaDebuggerEnabled) && _settings->GetDebugConfig().BreakOnUninitRead) {
					_step->Break(BreakSource::BreakOnUninitMemoryRead);
				}
			}
		}

		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation, addressInfo);
		}

		if(addr >= 0x04000000 && addr < 0x08000000) {
			_eventManager->AddEvent(DebugEventType::Register, operation);
		}

		_debugger->ProcessBreakConditions<accessWidth>(CpuType::Gba, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	}
}

template<uint8_t accessWidth>
void GbaDebugger::ProcessWrite(uint32_t addr, uint32_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _memoryManager->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::GbaMemory);
	InstructionProgress.LastMemOperation = operation;
	_debugger->ProcessBreakConditions<accessWidth>(CpuType::Gba, *_step.get(), _breakpointManager.get(), operation, addressInfo);

	switch(addressInfo.Type) {
		case MemoryType::GbaIntWorkRam:
		case MemoryType::GbaSaveRam:
		case MemoryType::GbaExtWorkRam:
		case MemoryType::GbaVideoRam:
		case MemoryType::GbaSpriteRam:
		case MemoryType::GbaPaletteRam:
			_disassembler->InvalidateCache(addressInfo, CpuType::Gba);
			break;
	}

	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation, addressInfo);
	}

	if(addr >= 0x04000000 && addr < 0x08000000) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	_memoryAccessCounter->ProcessMemoryWrite<accessWidth>(addressInfo, _console->GetMasterClock());
}

void GbaDebugger::Run()
{
	_step.reset(new StepRequest());
}

void GbaDebugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step(type);

	switch(type) {
		case StepType::Step: step.StepCount = stepCount; break;
		case StepType::StepOut: step.BreakAddress = _callstackManager->GetReturnAddress(); break;
		case StepType::StepOver:
			if(GbaDisUtils::IsJumpToSub(_prevOpCode, _prevFlags)) {
				step.BreakAddress = _prevProgramCounter + GbaDisUtils::GetOpSize(_prevOpCode, _prevFlags);
			} else {
				//For any other instruction, step over is the same as step into
				step.StepCount = 1;
			}
			break;

		case StepType::PpuStep: step.PpuStepCount = stepCount; break;
		case StepType::PpuScanline: step.PpuStepCount = 308*4 * stepCount; break;
		case StepType::PpuFrame: step.PpuStepCount = 308*4*228 * stepCount; break;
		case StepType::SpecificScanline: step.BreakScanline = stepCount; break;
	}

	_step.reset(new StepRequest(step));
}

StepBackConfig GbaDebugger::GetStepBackConfig()
{
	return {
		GetCpuCycleCount(),
		308 * 4,
		308 * 4 * 228
	};
}

void GbaDebugger::DrawPartialFrame()
{
	_ppu->DebugSendFrame();
}

void GbaDebugger::ProcessCallStackUpdates(AddressInfo& destAddr, uint32_t destPc)
{
	if(GbaDisUtils::IsJumpToSub(_prevOpCode, _prevFlags) && destPc != _prevProgramCounter + GbaDisUtils::GetOpSize(_prevOpCode, _prevFlags)) {
		//New PC doesn't match the next instruction, so the call was (probably) done
		uint32_t returnPc = _prevProgramCounter + GbaDisUtils::GetOpSize(_prevOpCode, _prevFlags);
		AddressInfo src = _memoryManager->GetAbsoluteAddress(_prevProgramCounter);
		AddressInfo ret = _memoryManager->GetAbsoluteAddress(returnPc);
		_callstackManager->Push(src, _prevProgramCounter, destAddr, destPc, ret, returnPc, 0, StackFrameFlags::None);
	} else if(GbaDisUtils::IsUnconditionalJump(_prevOpCode, _prevFlags) || GbaDisUtils::IsConditionalJump(_prevOpCode, _prevFlags)) {
		if(destPc != _prevProgramCounter + GbaDisUtils::GetOpSize(_prevOpCode, _prevFlags)) {
			//Return instruction used, and PC doesn't match the next instruction, so the ret was (probably) taken (can be conditional)
			if(_callstackManager->IsReturnAddrMatch(destPc)) {
				//Only pop top of callstack if the address matches the expected address
				_callstackManager->Pop(destAddr, destPc, 0);
			}
		}

		if(_step->BreakAddress == destPc) {
			//If we're on the expected return address, break immediately (for step over/step out)
			_step->Break(BreakSource::CpuStep);
		}
	}
}

void GbaDebugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	AddressInfo ret = _memoryManager->GetAbsoluteAddress(originalPc);
	AddressInfo dest = _memoryManager->GetAbsoluteAddress(currentPc);

	if(dest.Type == MemoryType::GbaPrgRom && dest.Address >= 0) {
		_codeDataLogger->SetCode(dest.Address, CdlFlags::SubEntryPoint);
	}

	//If a call/return occurred just before IRQ, it needs to be processed now
	ProcessCallStackUpdates(ret, originalPc);
	ResetPrevOpCode();

	_debugger->InternalProcessInterrupt(
		CpuType::Gba, *this, *_step.get(), 
		ret, originalPc, dest, currentPc, ret, originalPc, 0, forNmi
	);
}

void GbaDebugger::ProcessPpuCycle()
{
	if(_ppuTools->HasOpenedViewer()) {
		_ppuTools->UpdateViewers(_ppu->GetScanline(), _ppu->GetCycle());
	}

	if(_step->HasRequest) {
		if(_step->HasScanlineBreakRequest() && _ppu->GetCycle() == 0 && _ppu->GetScanline() == _step->BreakScanline) {
			_debugger->SleepUntilResume(CpuType::Gba, _step->GetBreakSource());
		} else if(_step->PpuStepCount > 0) {
			_step->PpuStepCount--;
			if(_step->PpuStepCount == 0) {
				_debugger->SleepUntilResume(CpuType::Gba, _step->GetBreakSource());
			}
		}
	}
}

DebuggerFeatures GbaDebugger::GetSupportedFeatures()
{
	DebuggerFeatures features = {};
	features.RunToIrq = true;
	features.RunToNmi = false;
	features.StepOver = true;
	features.StepOut = true;
	features.StepBack = true;
	features.CallStack = true;
	features.ChangeProgramCounter = AllowChangeProgramCounter;

	features.CpuVectorCount = 3;
	features.CpuVectors[0] = { "IRQ", (int)GbaCpuVector::Irq, VectorType::Direct };
	features.CpuVectors[1] = { "SWI", (int)GbaCpuVector::SoftwareIrq, VectorType::Direct };
	features.CpuVectors[2] = { "Undefined", (int)GbaCpuVector::Undefined, VectorType::Direct };

	return features;
}

void GbaDebugger::SetProgramCounter(uint32_t addr, bool updateDebuggerOnly)
{
	if(!updateDebuggerOnly) {
		AddressInfo absAddress = _memoryManager->GetAbsoluteAddress(addr);
		DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(absAddress, addr, _cpu->GetState().CPSR.ToInt32(), CpuType::Gba);
		_cpu->SetProgramCounter(addr, disInfo.GetFlags() & GbaCdlFlags::Thumb);
	}
	_prevOpCode = _memoryManager->DebugCpuRead(_cpu->GetState().CPSR.Thumb ? GbaAccessMode::HalfWord : GbaAccessMode::Word, addr);
	_prevProgramCounter = addr;
}

uint32_t GbaDebugger::GetProgramCounter(bool getInstPc)
{
	return getInstPc ? _prevProgramCounter : _cpu->GetState().R[15];
}

uint64_t GbaDebugger::GetCpuCycleCount(bool forProfiler)
{
	return _memoryManager->GetMasterClock();
}

void GbaDebugger::ResetPrevOpCode()
{
	_prevOpCode = 0;
}

uint8_t GbaDebugger::GetCpuFlags()
{
	return _cpu->GetState().CPSR.Thumb ? GbaCdlFlags::Thumb : 0;
}

BaseEventManager* GbaDebugger::GetEventManager()
{
	return _eventManager.get();
}

IAssembler* GbaDebugger::GetAssembler()
{
	return _assembler.get();
}

CallstackManager* GbaDebugger::GetCallstackManager()
{
	return _callstackManager.get();
}

BreakpointManager* GbaDebugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

BaseState& GbaDebugger::GetState()
{
	return _cpu->GetState();
}

void GbaDebugger::GetPpuState(BaseState& state)
{
	(GbaPpuState&)state = _ppu->GetState();
}

void GbaDebugger::SetPpuState(BaseState& srcState)
{
	GbaPpuState& dstState = _ppu->GetState();
	dstState = (GbaPpuState&)srcState;
}

ITraceLogger* GbaDebugger::GetTraceLogger()
{
	return _traceLogger.get();
}

PpuTools* GbaDebugger::GetPpuTools()
{
	return _ppuTools.get();
}

bool GbaDebugger::SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption)
{
	vector<uint8_t> output;

	uint8_t* prgRom = _debugger->GetMemoryDumper()->GetMemoryBuffer(MemoryType::GbaPrgRom);
	uint32_t prgRomSize = _debugger->GetMemoryDumper()->GetMemorySize(MemoryType::GbaPrgRom);
	vector<uint8_t> rom = vector<uint8_t>(prgRom, prgRom + prgRomSize);

	if(saveAsIps) {
		vector<uint8_t> originalRom;
		_emu->GetRomInfo().RomFile.ReadFile(originalRom);

		output = IpsPatcher::CreatePatch(originalRom, rom);
	} else {
		if(stripOption != CdlStripOption::StripNone) {
			vector<uint8_t> header(rom.begin(), rom.begin() + 0xC0);
			_codeDataLogger->StripData(rom.data(), stripOption);

			//Preserve rom header regardless of CDL file contents
			memcpy(rom.data(), header.data(), header.size());
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

void GbaDebugger::ProcessInputOverrides(DebugControllerState inputOverrides[8])
{
	BaseControlManager* controlManager = _console->GetControlManager();
	for(int i = 0; i < 8; i++) {
		shared_ptr<GbaController> controller = std::dynamic_pointer_cast<GbaController>(controlManager->GetControlDeviceByIndex(i));
		if(controller && inputOverrides[i].HasPressedButton()) {
			controller->SetBitValue(GbaController::Buttons::A, inputOverrides[i].A);
			controller->SetBitValue(GbaController::Buttons::B, inputOverrides[i].B);
			controller->SetBitValue(GbaController::Buttons::Select, inputOverrides[i].Select);
			controller->SetBitValue(GbaController::Buttons::Start, inputOverrides[i].Start);
			controller->SetBitValue(GbaController::Buttons::Up, inputOverrides[i].Up);
			controller->SetBitValue(GbaController::Buttons::Down, inputOverrides[i].Down);
			controller->SetBitValue(GbaController::Buttons::Left, inputOverrides[i].Left);
			controller->SetBitValue(GbaController::Buttons::Right, inputOverrides[i].Right);
			controller->SetBitValue(GbaController::Buttons::L, inputOverrides[i].L);
			controller->SetBitValue(GbaController::Buttons::R, inputOverrides[i].R);
		}
	}
	controlManager->RefreshHubState();
}

template void GbaDebugger::ProcessInstruction<2>();
template void GbaDebugger::ProcessInstruction<4>();

template void GbaDebugger::ProcessRead<1>(uint32_t addr, uint32_t value, MemoryOperationType type);
template void GbaDebugger::ProcessRead<2>(uint32_t addr, uint32_t value, MemoryOperationType type);
template void GbaDebugger::ProcessRead<4>(uint32_t addr, uint32_t value, MemoryOperationType type);

template void GbaDebugger::ProcessWrite<1>(uint32_t addr, uint32_t value, MemoryOperationType type);
template void GbaDebugger::ProcessWrite<2>(uint32_t addr, uint32_t value, MemoryOperationType type);
template void GbaDebugger::ProcessWrite<4>(uint32_t addr, uint32_t value, MemoryOperationType type);
