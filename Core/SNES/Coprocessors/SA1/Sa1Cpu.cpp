#include "pch.h"
#include "Utilities/Serializer.h"
#include "Shared/Emulator.h"
#include "SNES/SnesCpuTypes.h"
#include "SNES/Coprocessors/SA1/Sa1Cpu.h"
#include "SNES/SnesMemoryManager.h"
#include "SNES/Coprocessors/SA1/Sa1.h"
#include "SNES/MemoryMappings.h"
#include "MemoryOperationType.h"
#include "EventType.h"

#define SnesCpu Sa1Cpu
#include "SNES/SnesCpu.Instructions.h"
#include "SNES/SnesCpu.Shared.h"
#undef SnesCpu

Sa1Cpu::Sa1Cpu(Sa1* sa1, Emulator* emu)
{
	_sa1 = sa1;
	_emu = emu;
}

Sa1Cpu::~Sa1Cpu()
{
}

void Sa1Cpu::Exec()
{
	_immediateMode = false;

	switch(_state.StopState) {
		case SnesCpuStopState::Running:
			_emu->ProcessInstruction<CpuType::Sa1>();
			RunOp();
			break;

		case SnesCpuStopState::Stopped:
			//STP was executed, CPU no longer executes any code
			_state.CycleCount++;
			return;

		case SnesCpuStopState::WaitingForIrq:
			//WAI
			Idle();
			if(_state.IrqSource || _state.NeedNmi) {
				Idle();
				_state.StopState = SnesCpuStopState::Running;
			}
			break;
	}

	//Use the state of the IRQ/NMI flags on the previous cycle to determine if an IRQ is processed or not
	if(_state.PrevNeedNmi) {
		_state.NeedNmi = false;
		uint32_t originalPc = GetProgramAddress(_state.PC);
		ProcessInterrupt(_state.EmulationMode ? Sa1Cpu::LegacyNmiVector : Sa1Cpu::NmiVector, true);
		_emu->ProcessInterrupt<CpuType::Sa1>(originalPc, GetProgramAddress(_state.PC), true);
	} else if(_state.PrevIrqSource) {
		uint32_t originalPc = GetProgramAddress(_state.PC);
		ProcessInterrupt(_state.EmulationMode ? Sa1Cpu::LegacyIrqVector : Sa1Cpu::IrqVector, true);
		_emu->ProcessInterrupt<CpuType::Sa1>(originalPc, GetProgramAddress(_state.PC), false);
	}
}

void Sa1Cpu::Idle()
{
	//Do not apply any delay to internal cycles: "internal SA-1 cycles are still 10.74 MHz."
	_state.CycleCount++;
	DetectNmiSignalEdge();
	UpdateIrqNmiFlags();
	_emu->ProcessIdleCycle<CpuType::Sa1>();
}

void Sa1Cpu::IdleEndJump()
{
	IMemoryHandler* handler = _sa1->GetMemoryMappings()->GetHandler(_state.PC);
	if(handler && handler->GetMemoryType() == MemoryType::SnesPrgRom) {
		//Jumps/returns in PRG ROM take an extra cycle
		_state.CycleCount++;
		_emu->ProcessIdleCycle<CpuType::Sa1>();
		if(_sa1->GetSnesCpuMemoryType() == MemoryType::SnesPrgRom) {
			//Add an extra wait cycle if a conflict occurs at the same time
			_state.CycleCount++;
			_emu->ProcessIdleCycle<CpuType::Sa1>();
		}
	}
}

void Sa1Cpu::IdleTakeBranch()
{
	if(_state.PC & 0x01) {
		IMemoryHandler* handler = _sa1->GetMemoryMappings()->GetHandler(_state.PC);
		if(handler && handler->GetMemoryType() == MemoryType::SnesPrgRom) {
			//Branches to an odd address take an extra cycle
			_state.CycleCount++;
			_emu->ProcessIdleCycle<CpuType::Sa1>();
		}
	}
}

bool Sa1Cpu::IsAccessConflict()
{
	return _sa1->GetSnesCpuMemoryType() == _sa1->GetSa1MemoryType() && _sa1->GetSa1MemoryType() != MemoryType::SnesRegister;
}

void Sa1Cpu::ProcessCpuCycle(uint32_t addr)
{
	_state.CycleCount++;

	if(_sa1->GetSa1MemoryType() == MemoryType::SnesSaveRam) {
		//BWRAM (save ram) access takes 2 cycles
		_emu->ProcessIdleCycle<CpuType::Sa1>();
		_state.CycleCount++;
		if(IsAccessConflict()) {
			_emu->ProcessIdleCycle<CpuType::Sa1>();
			_state.CycleCount++;
			_emu->ProcessIdleCycle<CpuType::Sa1>();
			_state.CycleCount++;
		}
	} else if(IsAccessConflict()) {
		//Add a wait cycle when a conflict occurs between both CPUs
		_emu->ProcessIdleCycle<CpuType::Sa1>();
		_state.CycleCount++;
		if(_sa1->GetSa1MemoryType() == MemoryType::Sa1InternalRam && _sa1->IsSnesCpuFastRomSpeed()) {
			//If it's an IRAM access during FastROM access (speed = 6), add another wait cycle
			_emu->ProcessIdleCycle<CpuType::Sa1>();
			_state.CycleCount++;
		}
	}

	DetectNmiSignalEdge();
	UpdateIrqNmiFlags();
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

uint16_t Sa1Cpu::ReadVector(uint16_t vector)
{
	return _sa1->ReadVector(vector);
}

uint16_t Sa1Cpu::GetResetVector()
{
	return _sa1->ReadVector(Sa1Cpu::ResetVector);
}

void Sa1Cpu::IncreaseCycleCount(uint64_t cycleCount)
{
	_state.CycleCount += cycleCount;
}