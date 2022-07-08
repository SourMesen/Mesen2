#include "stdafx.h"
#include "Utilities/Serializer.h"
#include "Shared/Emulator.h"
#include "Shared/CheatManager.h"
#include "SNES/SnesCpuTypes.h"
#include "SNES/SnesCpu.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/SnesDmaController.h"
#include "SNES/SnesCpu.Instructions.h"
#include "SNES/SnesCpu.Shared.h"
#include "EventType.h"
#include "MemoryOperationType.h"

#ifndef DUMMYCPU
SnesCpu::SnesCpu(SnesConsole *console)
{
	_console = console;
	_emu = console->GetEmulator();
	_memoryManager = console->GetMemoryManager();
	_dmaController = console->GetDmaController();
}
#endif

SnesCpu::~SnesCpu()
{
}

void SnesCpu::Exec()
{
	_immediateMode = false;

	switch(_state.StopState) {
		case SnesCpuStopState::Running:
#ifndef DUMMYCPU
			_emu->ProcessInstruction<CpuType::Snes>();
#endif

			RunOp();
			break;

		case SnesCpuStopState::Stopped:
			//STP was executed, CPU no longer executes any code
		#ifndef DUMMYCPU
			_memoryManager->IncMasterClock4();
		#endif
			return;

		case SnesCpuStopState::WaitingForIrq:
			//WAI
			Idle();
			if(_state.IrqSource || _state.NeedNmi) {
				Idle();
				Idle();
				_state.StopState = SnesCpuStopState::Running;
			}
			break;
	}

#ifndef DUMMYCPU
	//Use the state of the IRQ/NMI flags on the previous cycle to determine if an IRQ is processed or not
	if(_state.PrevNeedNmi) {
		_state.NeedNmi = false;
		uint32_t originalPc = GetProgramAddress(_state.PC);
		_emu->GetCheatManager()->RefreshRamCheats(CpuType::Snes);
		ProcessInterrupt(_state.EmulationMode ? SnesCpu::LegacyNmiVector : SnesCpu::NmiVector, true);
		_emu->ProcessInterrupt<CpuType::Snes>(originalPc, GetProgramAddress(_state.PC), true);
	} else if(_state.PrevIrqSource) {
		uint32_t originalPc = GetProgramAddress(_state.PC);
		ProcessInterrupt(_state.EmulationMode ? SnesCpu::LegacyIrqVector : SnesCpu::IrqVector, true);
		_emu->ProcessInterrupt<CpuType::Snes>(originalPc, GetProgramAddress(_state.PC), false);
	}
#endif
}

void SnesCpu::Idle()
{
#ifndef DUMMYCPU
	_memoryManager->SetCpuSpeed(6);
	ProcessCpuCycle();
	_memoryManager->IncMasterClock6();
	_emu->ProcessIdleCycle<CpuType::Snes>();
	UpdateIrqNmiFlags();
#endif
}

void SnesCpu::IdleEndJump()
{
	//Used by SA1
}

void SnesCpu::IdleTakeBranch()
{
	//Used by SA1
}

void SnesCpu::ProcessCpuCycle()
{
	_state.CycleCount++;
	DetectNmiSignalEdge();
	_state.IrqLock = _dmaController->ProcessPendingTransfers();
}

uint16_t SnesCpu::ReadVector(uint16_t vector)
{
	//Overridden in SA-1 to return the correct value directly, rather than loading from ROM
	return ReadDataWord(vector);
}

uint16_t SnesCpu::GetResetVector()
{
	return _memoryManager->PeekWord(SnesCpu::ResetVector);
}

#ifndef DUMMYCPU
uint8_t SnesCpu::Read(uint32_t addr, MemoryOperationType type)
{
	_memoryManager->SetCpuSpeed(_memoryManager->GetCpuSpeed(addr));
	ProcessCpuCycle();
	uint8_t value = _memoryManager->Read(addr, type);
	UpdateIrqNmiFlags();
	return value;
}

void SnesCpu::Write(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	_memoryManager->SetCpuSpeed(_memoryManager->GetCpuSpeed(addr));
	ProcessCpuCycle();
	_memoryManager->Write(addr, value, type);
	UpdateIrqNmiFlags();
}
#endif
