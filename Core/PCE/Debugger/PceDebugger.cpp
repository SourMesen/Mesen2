#include "pch.h"
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
#include "Debugger/StepBackManager.h"
#include "PCE/PceConsole.h"
#include "PCE/PceCpu.h"
#include "PCE/PceVdc.h"
#include "PCE/PceVce.h"
#include "PCE/PceVpc.h"
#include "PCE/PceMemoryManager.h"
#include "PCE/Input/PceController.h"
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
#include "Shared/BaseControlManager.h"
#include "Shared/EmuSettings.h"
#include "Shared/SettingTypes.h"
#include "Shared/Emulator.h"
#include "Shared/MemoryOperationType.h"

PceDebugger::PceDebugger(Debugger* debugger) : IDebugger(debugger->GetEmulator())
{
	_debugger = debugger;
	_emu = debugger->GetEmulator();
	PceConsole* console = (PceConsole*)debugger->GetConsole();
	
	_console = console;
	_cpu = console->GetCpu();
	_vdc = console->GetVdc();
	_vce = console->GetVce();
	_vpc = console->GetVpc();
	_memoryManager = console->GetMemoryManager();

	_traceLogger.reset(new PceTraceLogger(debugger, this, _vdc));
	_ppuTools.reset(new PceVdcTools(debugger, debugger->GetEmulator(), console));
	_disassembler = debugger->GetDisassembler();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter();
	_settings = debugger->GetEmulator()->GetSettings();

	_dummyCpu.reset(new DummyPceCpu(_emu, _memoryManager));

	_codeDataLogger.reset(new CodeDataLogger(debugger, MemoryType::PcePrgRom, _emu->GetMemory(MemoryType::PcePrgRom).Size, CpuType::Pce, _emu->GetCrc32()));

	_cdlFile = _codeDataLogger->GetCdlFilePath(_console->GetRomFormat() == RomFormat::PceCdRom ? "PceCdromBios.cdl" : _emu->GetRomInfo().RomFile.GetFileName());
	_codeDataLogger->LoadCdlFile(_cdlFile, _settings->GetDebugConfig().AutoResetCdl);

	_stepBackManager.reset(new StepBackManager(_emu, this));
	_eventManager.reset(new PceEventManager(debugger, console));
	_callstackManager.reset(new CallstackManager(debugger, this));
	_breakpointManager.reset(new BreakpointManager(debugger, this, CpuType::Pce, _eventManager.get()));
	_step.reset(new StepRequest());
	_assembler.reset(new PceAssembler(_debugger->GetLabelManager()));
}

PceDebugger::~PceDebugger()
{
	_codeDataLogger->SaveCdlFile(_cdlFile);
}

void PceDebugger::Reset()
{
	_callstackManager->Clear();
	ResetPrevOpCode();
}

uint64_t PceDebugger::GetCpuCycleCount(bool forProfiler)
{
	return _cpu->GetState().CycleCount;
}

void PceDebugger::ResetPrevOpCode()
{
	_prevOpCode = 0x01;
}

void PceDebugger::ProcessInstruction()
{
	PceCpuState& state = _cpu->GetState();
	uint16_t pc = state.PC;
	uint8_t opCode = _memoryManager->DebugRead(pc);
	AddressInfo addressInfo = _memoryManager->GetAbsoluteAddress(pc);
	MemoryOperationInfo operation(pc, opCode, MemoryOperationType::ExecOpCode, MemoryType::PceMemory);
	InstructionProgress.LastMemOperation = operation;
	InstructionProgress.StartCycle = state.CycleCount;

	bool needDisassemble = _traceLogger->IsEnabled() || _settings->CheckDebuggerFlag(DebuggerFlags::PceDebuggerEnabled);
	if(addressInfo.Address >= 0) {
		if(addressInfo.Type == MemoryType::PcePrgRom) {
			_codeDataLogger->SetCode(addressInfo.Address, PceDisUtils::GetOpFlags(_prevOpCode, pc, _prevProgramCounter));
		}
		if(needDisassemble) {
			_disassembler->BuildCache(addressInfo, 0, CpuType::Pce);
		}
	}

	ProcessCallStackUpdates(addressInfo, pc, state.SP);

	_prevOpCode = opCode;
	_prevProgramCounter = pc;
	_prevStackPointer = state.SP;

	_step->ProcessCpuExec();

	if(_settings->CheckDebuggerFlag(DebuggerFlags::PceDebuggerEnabled)) {
		if(opCode == 0x00 && _settings->GetDebugConfig().PceBreakOnBrk) {
			_step->Break(BreakSource::BreakOnBrk);
		} else if(_settings->GetDebugConfig().PceBreakOnUnofficialOpCode && PceDisUtils::IsOpUnofficial(opCode)) {
			_step->Break(BreakSource::BreakOnUnofficialOpCode);
		}
	}
	
	if(_step->StepCount != 0 && _breakpointManager->HasBreakpoints() && _settings->GetDebugConfig().UsePredictiveBreakpoints) {
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
	InstructionProgress.LastMemOperation = operation;

	if(IsRegister(operation)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	if(type == MemoryOperationType::ExecOpCode) {
		if(_traceLogger->IsEnabled()) {
			PceCpuState& state = _cpu->GetState();
			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, state.PS, CpuType::Pce);
			_traceLogger->Log(state, disInfo, operation, addressInfo);
		}

		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetState().CycleCount);
		if(_step->ProcessCpuCycle()) {
			_debugger->SleepUntilResume(CpuType::Pce, BreakSource::CpuStep, &operation);
		}
	} else if(type == MemoryOperationType::ExecOperand) {
		if(addressInfo.Type == MemoryType::PcePrgRom && addressInfo.Address >= 0) {
			_codeDataLogger->SetCode(addressInfo.Address);
		}

		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation, addressInfo);
		}

		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _memoryManager->GetState().CycleCount);
		_step->ProcessCpuCycle();
		_debugger->ProcessBreakConditions(CpuType::Pce, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	} else {
		if(addressInfo.Type == MemoryType::PcePrgRom && addressInfo.Address >= 0 && operation.Type != MemoryOperationType::DummyRead) {
			_codeDataLogger->SetData(addressInfo.Address);
		}
		
		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation, addressInfo);
		}

		ReadResult result = _memoryAccessCounter->ProcessMemoryRead(addressInfo, _memoryManager->GetState().CycleCount);
		if(result != ReadResult::Normal && operation.Type != MemoryOperationType::DummyRead) {
			//Memory access was a read on an uninitialized memory address
			if(result == ReadResult::FirstUninitRead) {
				//Only warn the first time
				_debugger->Log("[CPU] Uninitialized memory read: $" + HexUtilities::ToHex((uint16_t)addr));
			}
			if(_settings->CheckDebuggerFlag(DebuggerFlags::PceDebuggerEnabled) && _settings->GetDebugConfig().BreakOnUninitRead) {
				_step->Break(BreakSource::BreakOnUninitMemoryRead);
			}
		}
		_step->ProcessCpuCycle();
		_debugger->ProcessBreakConditions(CpuType::Pce, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	}
}

void PceDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _memoryManager->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::PceMemory);
	InstructionProgress.LastMemOperation = operation;

	if(addressInfo.Address >= 0 && (addressInfo.Type == MemoryType::PceWorkRam || addressInfo.Type == MemoryType::PceCardRam || addressInfo.Type == MemoryType::PceCdromRam)) {
		_disassembler->InvalidateCache(addressInfo, CpuType::Pce);
	}

	if(IsRegister(operation)) {
		_eventManager->AddEvent(DebugEventType::Register, operation);
	}

	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation, addressInfo);
	}

	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _memoryManager->GetState().CycleCount);
	_step->ProcessCpuCycle();
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
		case StepType::StepOut:
			step.BreakAddress = _callstackManager->GetReturnAddress();
			step.BreakStackPointer = _callstackManager->GetReturnStackPointer();
			break;

		case StepType::StepOver:
			if(PceDisUtils::IsJumpToSub(_prevOpCode)) {
				step.BreakAddress = (_prevProgramCounter + PceDisUtils::GetOpSize(_prevOpCode)) & 0xFFFF;
				step.BreakStackPointer = _prevStackPointer;
			} else {
				//For any other instruction, step over is the same as step into
				step.StepCount = 1;
			}
			break;

		case StepType::CpuCycleStep: step.CpuCycleStepCount = stepCount; break;
		case StepType::PpuStep: step.PpuStepCount = stepCount; break;
		case StepType::PpuScanline: step.PpuStepCount = PceConstants::ClockPerScanline * stepCount; break;
		case StepType::PpuFrame: step.PpuStepCount = PceConstants::ClockPerScanline * _vce->GetScanlineCount() * stepCount; break;
		case StepType::SpecificScanline: step.BreakScanline = stepCount; break;
	}
	_step.reset(new StepRequest(step));
}

StepBackConfig PceDebugger::GetStepBackConfig()
{
	return {
		_memoryManager->GetState().CycleCount,
		PceConstants::ClockPerScanline,
		PceConstants::ClockPerScanline * _vce->GetScanlineCount()
	};
}

void PceDebugger::DrawPartialFrame()
{
	_vpc->DebugSendFrame();
}

void PceDebugger::ProcessCallStackUpdates(AddressInfo& destAddr, uint16_t destPc, uint8_t sp)
{
	if(PceDisUtils::IsJumpToSub(_prevOpCode)) {
		//JSR
		uint8_t opSize = PceDisUtils::GetOpSize(_prevOpCode);
		uint32_t returnPc = (_prevProgramCounter + opSize) & 0xFFFF;
		AddressInfo srcAddress = _memoryManager->GetAbsoluteAddress(_prevProgramCounter);
		AddressInfo retAddress = _memoryManager->GetAbsoluteAddress(returnPc);
		_callstackManager->Push(srcAddress, _prevProgramCounter, destAddr, destPc, retAddress, returnPc, _prevStackPointer, StackFrameFlags::None);
	} else if(PceDisUtils::IsReturnInstruction(_prevOpCode)) {
		//RTS, RTI
		_callstackManager->Pop(destAddr, destPc, sp);

		if(_step->BreakAddress == (int32_t)destPc && _step->BreakStackPointer == sp) {
			//RTS/RTI found, if we're on the expected return address, break immediately (for step over/step out)
			_step->Break(BreakSource::CpuStep);
		}
	}
}

void PceDebugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	AddressInfo ret = _memoryManager->GetAbsoluteAddress(originalPc);
	AddressInfo dest = _memoryManager->GetAbsoluteAddress(currentPc);
	
	if(dest.Type == MemoryType::PcePrgRom && dest.Address >= 0) {
		_codeDataLogger->SetCode(dest.Address, CdlFlags::SubEntryPoint);
	}

	uint8_t originalSp = _cpu->GetState().SP + 3;
	_prevStackPointer = originalSp;

	//If a call/return occurred just before IRQ, it needs to be processed now
	ProcessCallStackUpdates(ret, originalPc, originalSp);
	ResetPrevOpCode();

	_debugger->InternalProcessInterrupt(
		CpuType::Pce, *this, *_step.get(),
		ret, originalPc, dest, currentPc, ret, originalPc, originalSp, forNmi
	);
}

void PceDebugger::ProcessPpuRead(uint16_t addr, uint16_t value, MemoryType memoryType)
{
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Read, memoryType);
	AddressInfo addressInfo { addr, memoryType };
	_debugger->ProcessBreakConditions(CpuType::Pce, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryRead(addressInfo, _console->GetMasterClock());
}

void PceDebugger::ProcessPpuWrite(uint16_t addr, uint16_t value, MemoryType memoryType)
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
			_debugger->SleepUntilResume(CpuType::Pce, _step->GetBreakSource());
		} else if(_step->PpuStepCount > 0) {
			_step->PpuStepCount -= 3;
			if(_step->PpuStepCount <= 0) {
				_step->PpuStepCount = 0;
				_debugger->SleepUntilResume(CpuType::Pce, _step->GetBreakSource());
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
	features.StepBack = true;
	features.CallStack = true;
	features.ChangeProgramCounter = AllowChangeProgramCounter;
	features.CpuCycleStep = true;

	features.CpuVectors[0] = { "NMI", 0xFFFC };
	features.CpuVectors[1] = { "IRQ1", 0xFFF8 };
	features.CpuVectors[2] = { "IRQ2", 0xFFF6 };
	features.CpuVectors[3] = { "Timer", 0xFFFA };
	features.CpuVectors[4] = { "Reset", 0xFFFE };
	features.CpuVectorCount = 5;

	return features;
}

void PceDebugger::SetProgramCounter(uint32_t addr, bool updateDebuggerOnly)
{
	if(!updateDebuggerOnly) {
		_cpu->GetState().PC = (uint16_t)addr;
	}
	_prevOpCode = _memoryManager->DebugRead(addr);
	_prevProgramCounter = (uint16_t)addr;
	_prevStackPointer = _cpu->GetState().SP;
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
	_console->SetVideoState((PceVideoState&)state);
}

ITraceLogger* PceDebugger::GetTraceLogger()
{
	return _traceLogger.get();
}

PpuTools* PceDebugger::GetPpuTools()
{
	return _ppuTools.get();
}

bool PceDebugger::SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption)
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
		return true;
	}
	return false;
}

void PceDebugger::ProcessInputOverrides(DebugControllerState inputOverrides[8])
{
	BaseControlManager* controlManager = _console->GetControlManager();
	for(int i = 0; i < 8; i++) {
		shared_ptr<PceController> controller = std::dynamic_pointer_cast<PceController>(controlManager->GetControlDeviceByIndex(i));
		if(controller && inputOverrides[i].HasPressedButton()) {
			controller->SetBitValue(PceController::Buttons::I, inputOverrides[i].A);
			controller->SetBitValue(PceController::Buttons::II, inputOverrides[i].B);
			controller->SetBitValue(PceController::Buttons::Select, inputOverrides[i].Select);
			controller->SetBitValue(PceController::Buttons::Run, inputOverrides[i].Start);
			controller->SetBitValue(PceController::Buttons::Up, inputOverrides[i].Up);
			controller->SetBitValue(PceController::Buttons::Down, inputOverrides[i].Down);
			controller->SetBitValue(PceController::Buttons::Left, inputOverrides[i].Left);
			controller->SetBitValue(PceController::Buttons::Right, inputOverrides[i].Right);
		}
	}
	controlManager->RefreshHubState();
}
