#include "stdafx.h"
#include "../Utilities/Serializer.h"
#include "CpuTypes.h"
#include "Sa1Cpu.h"
#include "Console.h"
#include "MemoryManager.h"
#include "EventType.h"
#include "Sa1.h"

#define Cpu Sa1Cpu
#include "Cpu.Instructions.h"
#include "Cpu.Shared.h"
#undef Cpu

Sa1Cpu::Sa1Cpu(Sa1* sa1, Console* console)
{
	_sa1 = sa1;
	_console = console;
}

Sa1Cpu::~Sa1Cpu()
{
}

void Sa1Cpu::Exec()
{
	_immediateMode = false;

	switch(_state.StopState) {
		case CpuStopState::Running: RunOp(); break;
		case CpuStopState::Stopped:
			//STP was executed, CPU no longer executes any code
			_state.CycleCount++;
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

	//Use the state of the IRQ/NMI flags on the previous cycle to determine if an IRQ is processed or not
	if(_state.PrevNmiFlag) {
		uint32_t originalPc = GetProgramAddress(_state.PC);
		ProcessInterrupt(_state.EmulationMode ? Sa1Cpu::LegacyNmiVector : Sa1Cpu::NmiVector, true);
		_console->ProcessInterrupt<CpuType::Sa1>(originalPc, GetProgramAddress(_state.PC), true);
		_state.NmiFlag = false;
	} else if(_state.PrevIrqSource) {
		uint32_t originalPc = GetProgramAddress(_state.PC);
		ProcessInterrupt(_state.EmulationMode ? Sa1Cpu::LegacyIrqVector : Sa1Cpu::IrqVector, true);
		_console->ProcessInterrupt<CpuType::Sa1>(originalPc, GetProgramAddress(_state.PC), false);
	}
}

void Sa1Cpu::Idle()
{
	ProcessCpuCycle(0);
}

void Sa1Cpu::IdleEndJump()
{
	if(_sa1->GetSa1MemoryType() == SnesMemoryType::PrgRom) {
		//Jumps/returns in PRG ROM take an extra cycle
		_state.CycleCount++;
		if(IsAccessConflict()) {
			//Add an extra wait cycle if a conflict occurs at the same time
			_state.CycleCount++;
		}
	}
}

void Sa1Cpu::IdleTakeBranch()
{
	if(_sa1->GetSa1MemoryType() == SnesMemoryType::PrgRom && (_state.PC & 0x01)) {
		//Branches to an odd address take an extra cycle
		_state.CycleCount++;
	}
}

bool Sa1Cpu::IsAccessConflict()
{
	return _sa1->GetSnesCpuMemoryType() == _sa1->GetSa1MemoryType() && _sa1->GetSa1MemoryType() != SnesMemoryType::Register;
}

void Sa1Cpu::ProcessCpuCycle(uint32_t addr)
{
	_state.CycleCount++;

	if(_sa1->GetSa1MemoryType() == SnesMemoryType::SaveRam) {
		//BWRAM (save ram) access takes 2 cycles
		_state.CycleCount++;
		if(IsAccessConflict()) {
			_state.CycleCount += 2;
		}
	} else if(IsAccessConflict()) {
		//Add a wait cycle when a conflict occurs between both CPUs
		_state.CycleCount++;
		if(_sa1->GetSa1MemoryType() == SnesMemoryType::Sa1InternalRam) {
			//If it's an IRAM access, add another wait cycle
			_state.CycleCount++;
		}
	}

	_state.PrevNmiFlag = _state.NmiFlag;
	_state.PrevIrqSource = _state.IrqSource && !CheckFlag(ProcFlags::IrqDisable);
}

uint8_t Sa1Cpu::Read(uint32_t addr, MemoryOperationType type)
{
	ProcessCpuCycle(addr);
	return _sa1->ReadSa1(addr, type);
}

void Sa1Cpu::Write(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	ProcessCpuCycle(addr);
	_sa1->WriteSa1(addr, value, type);
}

uint16_t Sa1Cpu::GetResetVector()
{
	return _sa1->ReadSa1(Sa1Cpu::ResetVector) | (_sa1->ReadSa1(Sa1Cpu::ResetVector+1) << 8);
}
