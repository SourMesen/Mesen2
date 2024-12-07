#include "pch.h"
#include "WS/Debugger/DummyWsCpu.h"
#include "WS/Debugger/WsDebugger.h"
#include "WS/Debugger/WsAssembler.h"
#include "WS/Debugger/WsDisUtils.h"
#include "WS/Debugger/WsEventManager.h"
#include "WS/Debugger/WsTraceLogger.h"
#include "WS/Debugger/WsPpuTools.h"
#include "WS/WsController.h"
#include "WS/WsCpu.h"
#include "WS/WsPpu.h"
#include "WS/WsConsole.h"
#include "WS/WsMemoryManager.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Disassembler.h"
#include "Debugger/MemoryDumper.h"
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

WsDebugger::WsDebugger(Debugger* debugger) : IDebugger(debugger->GetEmulator())
{
	_debugger = debugger;
	_memoryDumper = debugger->GetMemoryDumper();
	_emu = debugger->GetEmulator();

	_disassembler = debugger->GetDisassembler();
	_memoryAccessCounter = debugger->GetMemoryAccessCounter();

	_console = ((WsConsole*)debugger->GetConsole());
	
	_cpu = _console->GetCpu();
	_ppu = _console->GetPpu();
	_memoryManager = _console->GetMemoryManager();

	_settings = debugger->GetEmulator()->GetSettings();
	
	_codeDataLogger.reset(new CodeDataLogger(debugger, MemoryType::WsPrgRom, _emu->GetMemory(MemoryType::WsPrgRom).Size, CpuType::Ws, _emu->GetCrc32()));
	_cdlFile = _codeDataLogger->GetCdlFilePath(_emu->GetRomInfo().RomFile.GetFileName());
	_codeDataLogger->LoadCdlFile(_cdlFile, _settings->GetDebugConfig().AutoResetCdl);

	_traceLogger.reset(new WsTraceLogger(debugger, this, _ppu));
	_ppuTools.reset(new WsPpuTools(debugger, debugger->GetEmulator(), _console));

	_stepBackManager.reset(new StepBackManager(_emu, this));
	_eventManager.reset(new WsEventManager(debugger, _console, _cpu, _ppu));
	_callstackManager.reset(new CallstackManager(debugger, this));
	_breakpointManager.reset(new BreakpointManager(debugger, this, CpuType::Ws, _eventManager.get()));
	_step.reset(new StepRequest());
	_assembler.reset(new WsAssembler(debugger->GetLabelManager()));
	
	_dummyCpu.reset(new DummyWsCpu(_emu, _memoryManager));

	ResetPrevOpCode();
}

WsDebugger::~WsDebugger()
{
	_codeDataLogger->SaveCdlFile(_cdlFile);
}

void WsDebugger::Reset()
{
	_callstackManager->Clear();
	ResetPrevOpCode();
}

void WsDebugger::ProcessInstruction()
{
	WsCpuState& state = _cpu->GetState();
	uint32_t pc = _cpu->GetProgramCounter(true);
	AddressInfo addressInfo = _console->GetAbsoluteAddress(pc);
	uint8_t value = _memoryManager->DebugRead(pc);
	MemoryOperationInfo operation(pc, value, MemoryOperationType::ExecOpCode, MemoryType::WsMemory);
	InstructionProgress.LastMemOperation = operation;
	InstructionProgress.StartCycle = state.CycleCount;

	if(addressInfo.Address >= 0) {
		if(addressInfo.Type == MemoryType::WsPrgRom) {
			if(WsDisUtils::IsConditionalJump(_prevOpCode)) {
				uint8_t opSize = WsDisUtils::GetOpSize(_prevProgramCounter, MemoryType::WsMemory, _memoryDumper);
				_codeDataLogger->SetCode(addressInfo.Address, WsDisUtils::GetOpFlags(_prevOpCode, pc, _prevProgramCounter, opSize));
			} else {
				_codeDataLogger->SetCode(addressInfo.Address, WsDisUtils::GetOpFlags(_prevOpCode, pc, _prevProgramCounter, 0));
			}
		}
		_disassembler->BuildCache(addressInfo, 0, CpuType::Ws);
	}

	ProcessCallStackUpdates(addressInfo, pc, state.SP);

	if(_step->BreakAddress == (int32_t)pc && _step->BreakStackPointer == state.SP) {
		//If we're on the expected return address, break immediately (for step over/step out)
		if(WsDisUtils::IsReturnInstruction(_prevOpCode)) {
			_step->Break(BreakSource::CpuStep);
		} else {
			uint8_t prefix = _memoryManager->DebugRead(_prevProgramCounter);
			if(prefix == 0xF2 || prefix == 0xF3) {
				//REPZ/REPNZ prefix
				_step->Break(BreakSource::CpuStep);
			}
		}
	}

	_prevOpCode = WsDisUtils::GetFullOpCode(state.CS, state.IP, _memoryManager);
	_prevProgramCounter = pc;
	_prevStackPointer = state.SP;

	if(_settings->CheckDebuggerFlag(DebuggerFlags::WsDebuggerEnabled)) {
		if(_settings->GetDebugConfig().WsBreakOnInvalidOpCode && WsDisUtils::IsUndefinedOpCode(_prevOpCode)) {
			_step->Break(BreakSource::BreakOnUndefinedOpCode);
		}
	}

	_step->ProcessCpuExec();

	if(_step->StepCount != 0 && _breakpointManager->HasBreakpoints() && _settings->GetDebugConfig().UsePredictiveBreakpoints) {
		_dummyCpu->SetDummyState(state);
		_dummyCpu->Exec();
		for(uint32_t i = 1; i < _dummyCpu->GetOperationCount(); i++) {
			MemoryOperationInfo memOp = _dummyCpu->GetOperationInfo(i);
			if(_breakpointManager->HasBreakpointForType(memOp.Type)) {
				AddressInfo absAddr = _console->GetAbsoluteAddress(memOp.Address);
				_debugger->ProcessPredictiveBreakpoint(CpuType::Ws, _breakpointManager.get(), memOp, absAddr);
			}
		}
	}

	_debugger->ProcessBreakConditions(CpuType::Ws, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

template<uint8_t accessWidth>
void WsDebugger::ProcessRead(uint32_t addr, uint16_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _console->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::WsMemory);
	InstructionProgress.LastMemOperation = operation;

	if(type == MemoryOperationType::ExecOpCode) {
		if(_traceLogger->IsEnabled()) {
			DisassemblyInfo disInfo = _disassembler->GetDisassemblyInfo(addressInfo, addr, 0, CpuType::Ws);
			_traceLogger->Log(_cpu->GetState(), disInfo, operation, addressInfo);
		}
		_memoryAccessCounter->ProcessMemoryExec(addressInfo, _console->GetMasterClock());
		if(_step->ProcessCpuCycle()) {
			_debugger->SleepUntilResume(CpuType::Ws, BreakSource::CpuStep, &operation);
		}
	} else if(type == MemoryOperationType::ExecOperand) {
		if(addressInfo.Address >= 0 && addressInfo.Type == MemoryType::WsPrgRom) {
			_codeDataLogger->SetCode(addressInfo.Address);
		}

		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation, addressInfo);
		}

		_memoryAccessCounter->ProcessMemoryExec<accessWidth>(addressInfo, _console->GetMasterClock());
		_step->ProcessCpuCycle();
		_debugger->ProcessBreakConditions(CpuType::Ws, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	} else {
		if(addressInfo.Address >= 0 && addressInfo.Type == MemoryType::WsPrgRom) {
			_codeDataLogger->SetData<0, accessWidth>(addressInfo.Address);
		}

		if(_traceLogger->IsEnabled()) {
			_traceLogger->LogNonExec(operation, addressInfo);
		}

		if(addr < 0xFE00 || addr >= 0xFF80) {
			ReadResult result = _memoryAccessCounter->ProcessMemoryRead<accessWidth>(addressInfo, _console->GetMasterClock());
			if(result != ReadResult::Normal) {
				//Memory access was a read on an uninitialized memory address
				if(result == ReadResult::FirstUninitRead) {
					//Only warn the first time
					_debugger->Log("[WS] Uninitialized memory read: $" + HexUtilities::ToHex20(addr));
				}
				if(_settings->CheckDebuggerFlag(DebuggerFlags::WsDebuggerEnabled) && _settings->GetDebugConfig().BreakOnUninitRead) {
					_step->Break(BreakSource::BreakOnUninitMemoryRead);
				}
			}
		}

		_step->ProcessCpuCycle();
		_debugger->ProcessBreakConditions<accessWidth>(CpuType::Ws, *_step.get(), _breakpointManager.get(), operation, addressInfo);
	}
}

template<uint8_t accessWidth>
void WsDebugger::ProcessWrite(uint32_t addr, uint16_t value, MemoryOperationType type)
{
	AddressInfo addressInfo = _console->GetAbsoluteAddress(addr);
	MemoryOperationInfo operation(addr, value, type, MemoryType::WsMemory);
	InstructionProgress.LastMemOperation = operation;

	if(addressInfo.Type == MemoryType::WsWorkRam || addressInfo.Type == MemoryType::WsCartRam) {
		if(addressInfo.Type == MemoryType::WsWorkRam) {
			bool isMono = _ppu->GetState().Mode == WsVideoMode::Monochrome;
			if(isMono && addressInfo.Address >= 0x2000 && addressInfo.Address <= 0x3FFF) {
				_eventManager->AddEvent(DebugEventType::Register, operation);
			} else if(!isMono && (addressInfo.Address >= 0xFE00 || (addressInfo.Address >= 0x4000 && addressInfo.Address <= 0xBFFF))) {
				_eventManager->AddEvent(DebugEventType::Register, operation);
			}
		}

		_disassembler->InvalidateCache(addressInfo, CpuType::Ws);
	}

	if(_traceLogger->IsEnabled()) {
		_traceLogger->LogNonExec(operation, addressInfo);
	}

	_memoryAccessCounter->ProcessMemoryWrite<accessWidth>(addressInfo, _console->GetMasterClock());
	_step->ProcessCpuCycle();
	_debugger->ProcessBreakConditions<accessWidth>(CpuType::Ws, *_step.get(), _breakpointManager.get(), operation, addressInfo);
}

template<MemoryOperationType opType, typename T>
void WsDebugger::ProcessMemoryAccess(uint32_t addr, T value, MemoryType memType)
{
	MemoryOperationInfo operation(addr, value, opType, memType);
	_eventManager->AddEvent(DebugEventType::Register, operation);
}

void WsDebugger::Run()
{
	_step.reset(new StepRequest());
}

void WsDebugger::Step(int32_t stepCount, StepType type)
{
	StepRequest step(type);

	switch(type) {
		case StepType::Step: step.StepCount = stepCount; break;
		case StepType::StepOut:
			step.BreakAddress = _callstackManager->GetReturnAddress();
			step.BreakStackPointer = _callstackManager->GetReturnStackPointer();
			break;
		case StepType::StepOver:
			if(WsDisUtils::IsJumpToSub(_prevOpCode)) {
				step.BreakAddress = _prevProgramCounter + GetPrevOpCodeSize();
				step.BreakStackPointer = _prevStackPointer;
			} else {
				uint8_t prefix = _memoryManager->DebugRead(_prevProgramCounter);
				if(prefix == 0xF2 || prefix == 0xF3) {
					//REPZ/REPNZ prefixed instruction
					step.BreakAddress = _prevProgramCounter + GetPrevOpCodeSize();
					step.BreakStackPointer = _prevStackPointer;
				} else {
					//For any other instruction, step over is the same as step into
					step.StepCount = 1;
				}
			}
			break;

		case StepType::CpuCycleStep: step.CpuCycleStepCount = stepCount; break;
		case StepType::PpuStep: step.PpuStepCount = stepCount; break;
		case StepType::PpuScanline: step.PpuStepCount = 256 * stepCount; break;
		case StepType::PpuFrame: step.PpuStepCount = 256 * _ppu->GetScanlineCount() * stepCount; break;
		case StepType::SpecificScanline: step.BreakScanline = stepCount; break;
	}

	_step.reset(new StepRequest(step));
}

StepBackConfig WsDebugger::GetStepBackConfig()
{
	return {
		_cpu->GetCycleCount(),
		256,
		256u * _ppu->GetScanlineCount()
	};
}

void WsDebugger::DrawPartialFrame()
{
	_ppu->DebugSendFrame();
}

uint8_t WsDebugger::GetPrevOpCodeSize()
{
	return WsDisUtils::GetOpSize(_prevProgramCounter, MemoryType::WsMemory, _debugger->GetMemoryDumper());
}

void WsDebugger::ProcessCallStackUpdates(AddressInfo& destAddr, uint32_t destPc, uint16_t sp)
{
	if(WsDisUtils::IsJumpToSub(_prevOpCode) && destPc != _prevProgramCounter + GetPrevOpCodeSize()) {
		//CALL and RST, and PC doesn't match the next instruction, so the call was (probably) done
		uint8_t opSize = GetPrevOpCodeSize();
		uint32_t returnPc = _prevProgramCounter + opSize;
		AddressInfo src = _console->GetAbsoluteAddress(_prevProgramCounter);
		AddressInfo ret = _console->GetAbsoluteAddress(returnPc);
		_callstackManager->Push(src, _prevProgramCounter, destAddr, destPc, ret, returnPc, _prevStackPointer, StackFrameFlags::None);
	} else if(WsDisUtils::IsReturnInstruction(_prevOpCode) && destPc != _prevProgramCounter + GetPrevOpCodeSize()) {
		//RET used, and PC doesn't match the next instruction, so the ret was (probably) taken
		_callstackManager->Pop(destAddr, destPc, sp);
	}
}

void WsDebugger::ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi)
{
	AddressInfo ret = _console->GetAbsoluteAddress(originalPc);
	AddressInfo dest = _console->GetAbsoluteAddress(currentPc);

	if(dest.Type == MemoryType::WsPrgRom && dest.Address >= 0) {
		_codeDataLogger->SetCode(dest.Address, CdlFlags::SubEntryPoint);
	}

	uint16_t originalSp = _cpu->GetState().SP + 6;
	_prevStackPointer = originalSp;

	//If a call/return occurred just before IRQ, it needs to be processed now
	ProcessCallStackUpdates(ret, originalPc, originalSp);
	ResetPrevOpCode();

	_debugger->InternalProcessInterrupt(
		CpuType::Ws, *this, *_step.get(), 
		ret, originalPc, dest, currentPc, ret, originalPc, originalSp, forNmi
	);
}

void WsDebugger::ProcessPpuCycle()
{
	if(_ppuTools->HasOpenedViewer()) {
		_ppuTools->UpdateViewers(_ppu->GetScanline(), _ppu->GetCycle());
	}

	if(_step->HasRequest) {
		if(_step->HasScanlineBreakRequest() && _ppu->GetCycle() == 0 && _ppu->GetScanline() == _step->BreakScanline) {
			_debugger->SleepUntilResume(CpuType::Ws, _step->GetBreakSource());
		} else if(_step->PpuStepCount > 0) {
			_step->PpuStepCount--;
			if(_step->PpuStepCount == 0) {
				_debugger->SleepUntilResume(CpuType::Ws, _step->GetBreakSource());
			}
		}
	}
}

DebuggerFeatures WsDebugger::GetSupportedFeatures()
{
	DebuggerFeatures features = {};
	features.RunToIrq = true;
	features.RunToNmi = false;
	features.StepOver = true;
	features.StepOut = true;
	features.StepBack = true;
	features.CallStack = true;
	features.CpuCycleStep = false;
	features.ChangeProgramCounter = AllowChangeProgramCounter;

	features.CpuVectors[0] = { "DIV0", 0, VectorType::x86 };
	features.CpuVectors[1] = { "TRAP", 1 * 4, VectorType::x86 };
	features.CpuVectors[2] = { "INT3", 3 * 4, VectorType::x86 };
	features.CpuVectors[3] = { "INTO", 4 * 4, VectorType::x86 };
	features.CpuVectors[4] = { "BOUND", 5 * 4, VectorType::x86 };
	
	features.IrqVectorOffset = _memoryManager->GetState().IrqVectorOffset;
	features.CpuVectors[5] = { "UART Send", 0, VectorType::x86WithOffset };
	features.CpuVectors[6] = { "Key Pressed", 1, VectorType::x86WithOffset };
	features.CpuVectors[7] = { "Cartridge", 2, VectorType::x86WithOffset };
	features.CpuVectors[8] = { "UART Receive", 3, VectorType::x86WithOffset };
	features.CpuVectors[9] = { "Scanline", 4, VectorType::x86WithOffset };
	features.CpuVectors[10] = { "V.Blank Timer", 5, VectorType::x86WithOffset };
	features.CpuVectors[11] = { "Vertical Blank", 6, VectorType::x86WithOffset };
	features.CpuVectors[12] = { "H.Blank Timer", 7, VectorType::x86WithOffset };
	features.CpuVectorCount = 13;

	return features;
}

void WsDebugger::SetProgramCounter(uint32_t addr, bool updateDebuggerOnly)
{
	if(!updateDebuggerOnly) {
		WsCpuState& state = _cpu->GetState();
		WsDisUtils::UpdateAddressCsIp(addr, state);
		_cpu->ClearPrefetch();
	}

	_prevOpCode = WsDisUtils::GetFullOpCode(_cpu->GetState().CS, _cpu->GetState().IP, _memoryManager);
	_prevProgramCounter = addr;
	_prevStackPointer = _cpu->GetState().SP;
}

uint32_t WsDebugger::GetProgramCounter(bool getInstPc)
{
	return getInstPc ? _prevProgramCounter : _cpu->GetProgramCounter();
}

uint64_t WsDebugger::GetCpuCycleCount(bool forProfiler)
{
	return _cpu->GetState().CycleCount;
}

void WsDebugger::ResetPrevOpCode()
{
	_prevOpCode = 0x90; //nop
}

BaseEventManager* WsDebugger::GetEventManager()
{
	return _eventManager.get();
}

IAssembler* WsDebugger::GetAssembler()
{
	return _assembler.get();
}

CallstackManager* WsDebugger::GetCallstackManager()
{
	return _callstackManager.get();
}

BreakpointManager* WsDebugger::GetBreakpointManager()
{
	return _breakpointManager.get();
}

BaseState& WsDebugger::GetState()
{
	return _cpu->GetState();
}

void WsDebugger::GetPpuState(BaseState& state)
{
	(WsPpuState&)state = _ppu->GetState();
}

void WsDebugger::SetPpuState(BaseState& srcState)
{
	WsPpuState& dstState = _ppu->GetState();
	dstState = (WsPpuState&)srcState;
}

ITraceLogger* WsDebugger::GetTraceLogger()
{
	return _traceLogger.get();
}

PpuTools* WsDebugger::GetPpuTools()
{
	return _ppuTools.get();
}

bool WsDebugger::SaveRomToDisk(string filename, bool saveAsIps, CdlStripOption stripOption)
{
	vector<uint8_t> output;

	uint8_t* prgRom = _debugger->GetMemoryDumper()->GetMemoryBuffer(MemoryType::WsPrgRom);
	uint32_t prgRomSize = _debugger->GetMemoryDumper()->GetMemorySize(MemoryType::WsPrgRom);
	vector<uint8_t> rom = vector<uint8_t>(prgRom, prgRom + prgRomSize);

	if(saveAsIps) {
		vector<uint8_t> originalRom;
		_emu->GetRomInfo().RomFile.ReadFile(originalRom);

		output = IpsPatcher::CreatePatch(originalRom, rom);
	} else {
		if(stripOption != CdlStripOption::StripNone) {
			_codeDataLogger->StripData(rom.data(), stripOption);

			//Preserve WS header
			memcpy(rom.data() + rom.size() - 0x10, prgRom + prgRomSize - 0x10, 0x10);
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

void WsDebugger::ProcessInputOverrides(DebugControllerState inputOverrides[8])
{
	BaseControlManager* controlManager = _console->GetControlManager();
	for(int i = 0; i < 8; i++) {
		shared_ptr<WsController> controller = std::dynamic_pointer_cast<WsController>(controlManager->GetControlDeviceByIndex(i));
		if(controller && inputOverrides[i].HasPressedButton()) {
			controller->SetBitValue(WsController::Buttons::A, inputOverrides[i].A);
			controller->SetBitValue(WsController::Buttons::B, inputOverrides[i].B);
			controller->SetBitValue(WsController::Buttons::Start, inputOverrides[i].Start);
			controller->SetBitValue(WsController::Buttons::Sound, inputOverrides[i].Select);
			controller->SetBitValue(WsController::Buttons::Up, inputOverrides[i].Up);
			controller->SetBitValue(WsController::Buttons::Down, inputOverrides[i].Down);
			controller->SetBitValue(WsController::Buttons::Left, inputOverrides[i].Left);
			controller->SetBitValue(WsController::Buttons::Right, inputOverrides[i].Right);
			controller->SetBitValue(WsController::Buttons::Up2, inputOverrides[i].U);
			controller->SetBitValue(WsController::Buttons::Down2, inputOverrides[i].D);
			controller->SetBitValue(WsController::Buttons::Left2, inputOverrides[i].L);
			controller->SetBitValue(WsController::Buttons::Right2, inputOverrides[i].R);
		}
	}
	controlManager->RefreshHubState();
}

template void WsDebugger::ProcessMemoryAccess<MemoryOperationType::Read>(uint32_t addr, uint8_t value, MemoryType memType);
template void WsDebugger::ProcessMemoryAccess<MemoryOperationType::Read>(uint32_t addr, uint16_t value, MemoryType memType);
template void WsDebugger::ProcessMemoryAccess<MemoryOperationType::Write>(uint32_t addr, uint8_t value, MemoryType memType);
template void WsDebugger::ProcessMemoryAccess<MemoryOperationType::Write>(uint32_t addr, uint16_t value, MemoryType memType);

template void WsDebugger::ProcessRead<1>(uint32_t addr, uint16_t value, MemoryOperationType type);
template void WsDebugger::ProcessRead<2>(uint32_t addr, uint16_t value, MemoryOperationType type);

template void WsDebugger::ProcessWrite<1>(uint32_t addr, uint16_t value, MemoryOperationType type);
template void WsDebugger::ProcessWrite<2>(uint32_t addr, uint16_t value, MemoryOperationType type);
