#include "pch.h"
#include "SMS/Debugger/DummySmsCpu.h"
#include "SMS/Debugger/SmsDebugger.h"
#include "SMS/Debugger/SmsAssembler.h"
#include "SMS/Debugger/SmsDisUtils.h"
#include "SMS/Debugger/SmsEventManager.h"
#include "SMS/Debugger/SmsTraceLogger.h"
#include "SMS/Debugger/SmsVdpTools.h"
#include "SMS/Input/SmsController.h"
#include "SMS/SmsCpu.h"
#include "SMS/SmsVdp.h"
#include "SMS/SmsConsole.h"
#include "SMS/SmsMemoryManager.h"
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

SmsDebugger::SmsDebugger(Debugger* debugger) : IDebugger(debugger->GetEmulator())
{
	_debugger = debugger;
	_emu = debugger->GetEmulator();

	_disassembler = debugger->GetDisassembler();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter();

	_console = ((SmsConsole*)debugger->GetConsole());
	
	_cpu = _console->GetCpu();
	_vdp = _console->GetVdp();
	_memoryManager = _console->GetMemoryManager();

	_settings = debugger->GetEmulator()->GetSettings();
	
	_codeDataLogger.reset(new CodeDataLogger(debugger, MemoryType::SmsPrgRom, _emu->GetMemory(MemoryType::SmsPrgRom).Size, CpuType::Sms, _emu->GetCrc32()));
	_cdlFile = _codeDataLogger->GetCdlFilePath(_emu->GetRomInfo().RomFile.GetFileName());
	_codeDataLogger->LoadCdlFile(_cdlFile, _settings->GetDebugConfig().AutoResetCdl);

	_traceLogger.reset(new SmsTraceLogger(debugger, this, _vdp));
	_ppuTools.reset(new SmsVdpTools(debugger, debugger->GetEmulator(), _console));

	_stepBackManager.reset(new StepBackManager(_emu, this));
	_eventManager.reset(new SmsEventManager(debugger, _console, _cpu, _vdp));
	_callstackManager.reset(new CallstackManager(debugger, this));
	_breakpointManager.reset(new BreakpointManager(debugger, this, CpuType::Sms, _eventManager.get()));
	_step.reset(new StepRequest());
	_assembler.reset(new SmsAssembler(debugger->GetLabelManager()));
	
	_dummyCpu.reset(new DummySmsCpu());
	_dummyCpu->Init(_emu, _console, _console->GetMemoryManager());
}

SmsDebugger::~SmsDebugger()
{
	_codeDataLogger->SaveCdlFile(_cdlFile);
}

void SmsDebugger::Reset()
{
	_callstackManager->Clear();
	ResetPrevOpCode();
}

void SmsDebugger::ProcessInstruction()
{
	SmsCpuState& state = _cpu->GetState();
	uint16_t pc = state.PC;
	AddressInfo addressInfo = _console->GetAbsoluteAddress(pc);
	uint8_t value = _memoryManager->DebugRead(pc);
	MemoryOperationInfo operation(pc, value, MemoryOperationType::ExecOpCode, MemoryType::SmsMemory);
	InstructionProgress.LastMemOperation = operation;
	InstructionProgress.StartCycle = state.CycleCount;

	if(addressInfo.Address >= 0) {
		if(addressInfo.Type == MemoryType::SmsPrgRom) {
			_codeDataLogger->SetCode(addressInfo.Address, SmsDisUtils::GetOpFlags(_prevOpCode, pc, _prevProgramCounter));
		}
		_disassembler->BuildCache(addressInfo, 0, CpuType::Sms);
	}

	ProcessCallStackUpdates(addressInfo, pc, state.SP);

	if(_settings->CheckDebuggerFlag(DebuggerFlags::SmsDebuggerEnabled)) {
		if(value == 0x40 && _settings->GetDebugConfig().SmsBreakOnNopLoad) {
			//Break on ld b, b
			_step->Break(BreakSource::SmsNopLoad);
		}
	}

	if(value == 0xED) {
		_prevOpCode = 0xED | (_memoryManager->DebugRead(pc + 1) << 8);
	} else {
		_prevOpCode = value;
	}
	_prevProgramCounter = pc;
	_prevStackPointer = state.SP;

	_step->ProcessCpuExec();

	if(_step->StepCount != 0 && _breakpointManager->HasBreakpoints() && _settings->GetDebugConfig().UsePredictiveBreakpoints) {
		_dummyCpu->SetDummyState(state);
		_dummyCpu->Exec();
		for(uint32_t i = 1; i < _dummyCpu->GetOperationCount(); i++) {
			MemoryOperationInfo memOp = _dummyCpu->GetOperationInfo(i);
			if(_breakpointManager->HasBreakpointForType(memOp.Type)) {
				AddressInfo absAddr = _console->GetAbsoluteAddress(memOp.Address);
				_debugger->ProcessPredictiveBreakpoint(CpuType::Sms, _breakpointManager.get(), memOp, absAddr);
			}
		}
	}

	_debugger->ProcessBreakConditions(CpuType::Sms, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

void SmsDebugger::ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _console->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::SmsMemory);
	InstructionProgress.LastMemOperation = operation;

	if(type == MemoryOperationType::ExecOpCode) {
		if(_traceLogger->IsEnabled()) {
			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::Sms);
			_traceLogger->Log(_cpu->GetState(), disInfo, operation, addressInfo);
		}
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _console->GetMasterClock());
		if(_step->ProcessCpuCycle()) {
			_debugger->SleepUntilResume(CpuType::Sms, BreakSource::CpuStep, &operation);
		}
	} else if(type == MemoryOperationType::ExecOperand) {
		if(addressInfo.Address >= 0 && addressInfo.Type == MemoryType::SmsPrgRom) {
			_codeDataLogger->SetCode(addressInfo.Address);
		}

		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation, addressInfo);
		}

		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _console->GetMasterClock());
		_step->ProcessCpuCycle();
		_debugger->ProcessBreakConditions(CpuType::Sms, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	} else {
		if(addressInfo.Address >= 0 && addressInfo.Type == MemoryType::SmsPrgRom) {
			_codeDataLogger->SetData(addressInfo.Address);
		}

		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation, addressInfo);
		}

		if(addr < 0xFE00 || addr >= 0xFF80) {
			ReadResult result = _memoryAccessCounter->ProcessMemoryRead(addressInfo, _console->GetMasterClock());
			if(result != ReadResult::Normal) {
				//Memory access was a read on an uninitialized memory address
				if(result == ReadResult::FirstUninitRead) {
					//Only warn the first time
					_debugger->Log("[SMS] Uninitialized memory read: $" + HexUtilities::ToHex((uint16_t)addr));
				}
				if(_settings->CheckDebuggerFlag(DebuggerFlags::SmsDebuggerEnabled) && _settings->GetDebugConfig().BreakOnUninitRead) {
					_step->Break(BreakSource::BreakOnUninitMemoryRead);
				}
			}
		}

		_step->ProcessCpuCycle();
		_debugger->ProcessBreakConditions(CpuType::Sms, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	}
}

void SmsDebugger::ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _console->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::SmsMemory);
	InstructionProgress.LastMemOperation = operation;

	if(addressInfo.Type == MemoryType::SmsWorkRam || addressInfo.Type == MemoryType::SmsCartRam) {
		_disassembler->InvalidateCache(addressInfo, CpuType::Sms);
	}

	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation, addressInfo);
	}

	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _console->GetMasterClock());
	_step->ProcessCpuCycle();
	_debugger->ProcessBreakConditions(CpuType::Sms, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

template<MemoryOperationType opType>
void SmsDebugger::ProcessMemoryAccess(uint32_t addr, uint8_t value, MemoryType memType)
{
	MemoryOperationInfo operation(addr, value, opType, memType);
	_eventManager->AddEvent(DebugEventType::Register, operation);
}

void SmsDebugger::Run()
{
	_step.reset(new StepRequest());
}

void SmsDebugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step(type);

	switch(type) {
		case StepType::Step: step.StepCount = stepCount; break;
		case StepType::StepOut:
			step.BreakAddress = _callstackManager->GetReturnAddress();
			step.BreakStackPointer = _callstackManager->GetReturnStackPointer();
			break;

		case StepType::StepOver:
			if(SmsDisUtils::IsJumpToSub(_prevOpCode)) {
				step.BreakAddress = _prevProgramCounter + GetPrevOpCodeSize();
				step.BreakStackPointer = _prevStackPointer;
			} else {
				//For any other instruction, step over is the same as step into
				step.StepCount = 1;
			}
			break;

		case StepType::CpuCycleStep: step.CpuCycleStepCount = stepCount; break;
		case StepType::PpuStep: step.PpuStepCount = stepCount; break;
		case StepType::PpuScanline: step.PpuStepCount = 342 * stepCount; break;
		case StepType::PpuFrame: step.PpuStepCount = 342 * _vdp->GetScanlineCount() * stepCount; break;
		case StepType::SpecificScanline: step.BreakScanline = stepCount; break;
	}

	_step.reset(new StepRequest(step));
}

StepBackConfig SmsDebugger::GetStepBackConfig()
{
	return {
		_cpu->GetCycleCount() * 3,
		342 * 2,
		342u * 2 * _vdp->GetScanlineCount()
	};
}

void SmsDebugger::DrawPartialFrame()
{
	_vdp->DebugSendFrame();
}

uint8_t SmsDebugger::GetPrevOpCodeSize()
{
	return SmsDisUtils::GetOpSize(_prevOpCode, _prevProgramCounter, MemoryType::SmsMemory, _debugger->GetMemoryDumper());
}

void SmsDebugger::ProcessCallStackUpdates(AddressInfo& destAddr, uint16_t destPc, uint16_t sp)
{
	if(SmsDisUtils::IsJumpToSub(_prevOpCode) && destPc != _prevProgramCounter + GetPrevOpCodeSize()) {
		//CALL and RST, and PC doesn't match the next instruction, so the call was (probably) done
		uint8_t opSize = GetPrevOpCodeSize();
		uint16_t returnPc = _prevProgramCounter + opSize;
		AddressInfo src = _console->GetAbsoluteAddress(_prevProgramCounter);
		AddressInfo ret = _console->GetAbsoluteAddress(returnPc);
		_callstackManager->Push(src, _prevProgramCounter, destAddr, destPc, ret, returnPc, _prevStackPointer, StackFrameFlags::None);
	} else if(SmsDisUtils::IsReturnInstruction(_prevOpCode)) {
		if(destPc != _prevProgramCounter + GetPrevOpCodeSize()) {
			//RET used, and PC doesn't match the next instruction, so the ret was (probably) taken
			_callstackManager->Pop(destAddr, destPc, sp);
		}

		if(_step->BreakAddress == (int32_t)destPc && _step->BreakStackPointer == sp) {
			//RET/RETI - if we're on the expected return address, break immediately (for step over/step out)
			_step->Break(BreakSource::CpuStep);
		}
	}
}

void SmsDebugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	AddressInfo ret = _console->GetAbsoluteAddress(originalPc);
	AddressInfo dest = _console->GetAbsoluteAddress(currentPc);

	if(dest.Type == MemoryType::SmsPrgRom && dest.Address >= 0) {
		_codeDataLogger->SetCode(dest.Address, CdlFlags::SubEntryPoint);
	}

	uint16_t originalSp = _cpu->GetState().SP + 2;
	_prevStackPointer = originalSp;

	//If a call/return occurred just before IRQ, it needs to be processed now
	ProcessCallStackUpdates(ret, originalPc, originalSp);
	ResetPrevOpCode();

	_debugger->InternalProcessInterrupt(
		CpuType::Sms, *this, *_step.get(), 
		ret, originalPc, dest, currentPc, ret, originalPc, originalSp, forNmi
	);
}

void SmsDebugger::ProcessPpuRead(uint16_t addr, uint8_t value, MemoryType memoryType)
{
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Read, memoryType);
	AddressInfo addressInfo { addr, memoryType };
	_debugger->ProcessBreakConditions(CpuType::Sms, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryRead(addressInfo, _console->GetMasterClock());
}

void SmsDebugger::ProcessPpuWrite(uint16_t addr, uint8_t value, MemoryType memoryType)
{
	MemoryOperationInfo operation(addr, value, MemoryOperationType::Write, memoryType);
	AddressInfo addressInfo { addr, memoryType };
	_debugger->ProcessBreakConditions(CpuType::Sms, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	_memoryAccessCounter->ProcessMemoryWrite(addressInfo, _console->GetMasterClock());
}

void SmsDebugger::ProcessPpuCycle()
{
	if(_ppuTools->HasOpenedViewer()) {
		_ppuTools->UpdateViewers(_vdp->GetScanline(), _vdp->GetCycle());
	}

	if(_step->HasRequest) {
		if(_step->HasScanlineBreakRequest() && _vdp->GetCycle() == 0 && _vdp->GetScanline() == _step->BreakScanline) {
			_debugger->SleepUntilResume(CpuType::Sms, _step->GetBreakSource());
		} else if(_step->PpuStepCount > 0) {
			_step->PpuStepCount--;
			if(_step->PpuStepCount == 0) {
				_debugger->SleepUntilResume(CpuType::Sms, _step->GetBreakSource());
			}
		}
	}
}

DebuggerFeatures SmsDebugger::GetSupportedFeatures()
{
	DebuggerFeatures features = {};
	features.RunToIrq = true;
	features.RunToNmi = true;
	features.StepOver = true;
	features.StepOut = true;
	features.StepBack = true;
	features.CallStack = true;
	features.CpuCycleStep = true;
	features.ChangeProgramCounter = AllowChangeProgramCounter;

	features.CpuVectors[0] = { "IRQ", 0x38, VectorType::Direct };
	features.CpuVectors[1] = { "NMI", 0x66, VectorType::Direct };
	features.CpuVectorCount = 2;

	return features;
}

void SmsDebugger::SetProgramCounter(uint32_t addr, bool updateDebuggerOnly)
{
	if(!updateDebuggerOnly) {
		_cpu->GetState().PC = (uint16_t)addr;
	}
	_prevOpCode = _memoryManager->DebugRead((uint16_t)addr);
	_prevProgramCounter = (uint16_t)addr;
	_prevStackPointer = _cpu->GetState().SP;
}

uint32_t SmsDebugger::GetProgramCounter(bool getInstPc)
{
	return getInstPc ? _prevProgramCounter : _cpu->GetState().PC;
}

uint64_t SmsDebugger::GetCpuCycleCount(bool forProfiler)
{
	return _cpu->GetCycleCount();
}

void SmsDebugger::ResetPrevOpCode()
{
	_prevOpCode = 0;
}

BaseEventManager* SmsDebugger::GetEventManager()
{
	return _eventManager.get();
}

IAssembler* SmsDebugger::GetAssembler()
{
	return _assembler.get();
}

CallstackManager* SmsDebugger::GetCallstackManager()
{
	return _callstackManager.get();
}

BreakpointManager* SmsDebugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

BaseState& SmsDebugger::GetState()
{
	return _cpu->GetState();
}

void SmsDebugger::GetPpuState(BaseState& state)
{
	(SmsVdpState&)state = _vdp->GetState();
}

void SmsDebugger::SetPpuState(BaseState& srcState)
{
	SmsVdpState& dstState = _vdp->GetState();
	dstState = (SmsVdpState&)srcState;
}

ITraceLogger* SmsDebugger::GetTraceLogger()
{
	return _traceLogger.get();
}

PpuTools* SmsDebugger::GetPpuTools()
{
	return _ppuTools.get();
}

bool SmsDebugger::SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption)
{
	vector<uint8_t> output;

	uint8_t* prgRom = _debugger->GetMemoryDumper()->GetMemoryBuffer(MemoryType::SmsPrgRom);
	uint32_t prgRomSize = _debugger->GetMemoryDumper()->GetMemorySize(MemoryType::SmsPrgRom);
	vector<uint8_t> rom = vector<uint8_t>(prgRom, prgRom + prgRomSize);

	if(saveAsIps) {
		vector<uint8_t> originalRom;
		_emu->GetRomInfo().RomFile.ReadFile(originalRom);

		output = IpsPatcher::CreatePatch(originalRom, rom);
	} else {
		if(stripOption != CdlStripOption::StripNone) {
			_codeDataLogger->StripData(rom.data(), stripOption);
			if(prgRomSize >= 0x8000) {
				memcpy(rom.data() + 0x7FF0, prgRom + 0x7FF0, 16);
			}
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

void SmsDebugger::ProcessInputOverrides(DebugControllerState inputOverrides[8])
{
	BaseControlManager* controlManager = _console->GetControlManager();
	for(int i = 0; i < 8; i++) {
		shared_ptr<SmsController> controller = std::dynamic_pointer_cast<SmsController>(controlManager->GetControlDeviceByIndex(i));
		if(controller && inputOverrides[i].HasPressedButton()) {
			controller->SetBitValue(SmsController::Buttons::A, inputOverrides[i].A);
			controller->SetBitValue(SmsController::Buttons::B, inputOverrides[i].B);
			controller->SetBitValue(SmsController::Buttons::Pause, inputOverrides[i].Start);
			controller->SetBitValue(SmsController::Buttons::Up, inputOverrides[i].Up);
			controller->SetBitValue(SmsController::Buttons::Down, inputOverrides[i].Down);
			controller->SetBitValue(SmsController::Buttons::Left, inputOverrides[i].Left);
			controller->SetBitValue(SmsController::Buttons::Right, inputOverrides[i].Right);
		}
	}
	controlManager->RefreshHubState();
}

template void SmsDebugger::ProcessMemoryAccess<MemoryOperationType::Read>(uint32_t addr, uint8_t value, MemoryType memType);
template void SmsDebugger::ProcessMemoryAccess<MemoryOperationType::Write>(uint32_t addr, uint8_t value, MemoryType memType);
