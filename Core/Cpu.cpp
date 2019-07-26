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

Cpu::Cpu(Console *console)
{
	_console = console;
	_memoryManager = console->GetMemoryManager().get();
	_dmaController = console->GetDmaController().get();
}

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
			if(!_state.IrqSource && !_state.NmiFlag) {
				Idle();
			} else {
				Idle();
				Idle();
				_state.StopState = CpuStopState::Running;
			}
			break;
	}

#ifndef DUMMYCPU
	//Use the state of the IRQ/NMI flags on the previous cycle to determine if an IRQ is processed or not
	if(_state.PrevNmiFlag) {
		uint32_t originalPc = GetProgramAddress(_state.PC);
		ProcessInterrupt(_state.EmulationMode ? Cpu::LegacyNmiVector : Cpu::NmiVector, true);
		_console->ProcessInterrupt<CpuType::Cpu>(originalPc, GetProgramAddress(_state.PC), true);
		_state.NmiFlag = false;
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

	bool irqLock = false;
	if(_dmaController->ProcessPendingTransfers()) {
		//If we just finished processing a DMA transfer, ignore the NMI/IRQ flags for this cycle
		irqLock = true;
	}

	if(!irqLock) {
		_state.PrevNmiFlag = _state.NmiFlag;
		_state.PrevIrqSource = _state.IrqSource && !CheckFlag(ProcFlags::IrqDisable);
	}
}

uint8_t Cpu::Read(uint32_t addr, MemoryOperationType type)
{
#ifdef DUMMYCPU
	uint8_t value = _memoryManager->Peek(addr);
	LogRead(addr, value);
	return value;
#else
	_memoryManager->SetCpuSpeed(_memoryManager->GetCpuSpeed(addr));
	ProcessCpuCycle();
	return _memoryManager->Read(addr, type);
#endif
}

void Cpu::Write(uint32_t addr, uint8_t value, MemoryOperationType type)
{
#ifdef DUMMYCPU
	LogWrite(addr, value);
#else
	_memoryManager->SetCpuSpeed(_memoryManager->GetCpuSpeed(addr));
	ProcessCpuCycle();
	_memoryManager->Write(addr, value, type);
#endif
}

uint16_t Cpu::GetResetVector()
{
	return _memoryManager->PeekWord(Cpu::ResetVector);
}
