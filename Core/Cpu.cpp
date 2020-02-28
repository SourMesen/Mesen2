#include "stdafx.h"
#include "../Utilities/Serializer.h"
#include "CpuTypes.h"
#include "Cpu.h"
#include "Console.h"
#include "MemoryManager.h"
#include "DmaController.h"
#include "EventType.h"
#include "Cpu.Instructions.h"
#include "Cpu.Shared.h"

#ifndef DUMMYCPU
Cpu::Cpu(Console *console)
{
	_console = console;
	_memoryManager = console->GetMemoryManager().get();
	_dmaController = console->GetDmaController().get();
}
#endif

Cpu::~Cpu()
{
}

void Cpu::Exec()
{
	_immediateMode = false;

	switch(_state.StopState) {
		case CpuStopState::Running: RunOp(); break;
		case CpuStopState::Stopped:
			//STP was executed, CPU no longer executes any code
		#ifndef DUMMYCPU
			_memoryManager->IncMasterClock4();
		#endif
			return;

		case CpuStopState::WaitingForIrq:
			//WAI
			Idle();
			if(_state.IrqSource || _state.NeedNmi) {
				Idle();
				Idle();
				_state.StopState = CpuStopState::Running;
			}
			break;
	}

#ifndef DUMMYCPU
	//Use the state of the IRQ/NMI flags on the previous cycle to determine if an IRQ is processed or not
	if(_state.PrevNeedNmi) {
		_state.NeedNmi = false;
		uint32_t originalPc = GetProgramAddress(_state.PC);
		ProcessInterrupt(_state.EmulationMode ? Cpu::LegacyNmiVector : Cpu::NmiVector, true);
		_console->ProcessInterrupt<CpuType::Cpu>(originalPc, GetProgramAddress(_state.PC), true);
	} else if(_state.PrevIrqSource) {
		uint32_t originalPc = GetProgramAddress(_state.PC);
		ProcessInterrupt(_state.EmulationMode ? Cpu::LegacyIrqVector : Cpu::IrqVector, true);
		_console->ProcessInterrupt<CpuType::Cpu>(originalPc, GetProgramAddress(_state.PC), false);
	}
#endif
}

void Cpu::Idle()
{
#ifndef DUMMYCPU
	_memoryManager->SetCpuSpeed(6);
	ProcessCpuCycle();
	_memoryManager->IncMasterClock6();
	UpdateIrqNmiFlags();
#endif
}

void Cpu::IdleEndJump()
{
	//Used by SA1
}

void Cpu::IdleTakeBranch()
{
	//Used by SA1
}

void Cpu::ProcessCpuCycle()
{
	_state.CycleCount++;
	DetectNmiSignalEdge();
	_state.IrqLock = _dmaController->ProcessPendingTransfers();
}

#ifndef DUMMYCPU
uint8_t Cpu::Read(uint32_t addr, MemoryOperationType type)
{
	_memoryManager->SetCpuSpeed(_memoryManager->GetCpuSpeed(addr));
	ProcessCpuCycle();
	uint8_t value = _memoryManager->Read(addr, type);
	UpdateIrqNmiFlags();
	return value;
}

void Cpu::Write(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	_memoryManager->SetCpuSpeed(_memoryManager->GetCpuSpeed(addr));
	ProcessCpuCycle();
	_memoryManager->Write(addr, value, type);
	UpdateIrqNmiFlags();
}
#endif

uint16_t Cpu::GetResetVector()
{
	return _memoryManager->PeekWord(Cpu::ResetVector);
}
