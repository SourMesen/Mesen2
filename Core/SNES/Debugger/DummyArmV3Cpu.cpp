#include "pch.h"
#include "SNES/Debugger/DummyArmV3Cpu.h"

#define DUMMYCPU
#define ArmV3Cpu DummyArmV3Cpu
#include "SNES/Coprocessors/ST018/ArmV3Cpu.cpp"
#undef ArmV3Cpu
#undef DUMMYCPU

void DummyArmV3Cpu::SetDummyState(ArmV3CpuState& state)
{
	_memOpCounter = 0;
	_state = state;
}

uint32_t DummyArmV3Cpu::GetOperationCount()
{
	return _memOpCounter;
}

void DummyArmV3Cpu::LogMemoryOperation(uint32_t addr, uint32_t value, ArmV3AccessModeVal mode, MemoryOperationType type)
{
	_memOperations[_memOpCounter] = {
		addr,
		(int32_t)value,
		type,
		MemoryType::St018Memory
	};
	_memAccessMode[_memOpCounter] = mode;
	_memOpCounter++;
}

MemoryOperationInfo DummyArmV3Cpu::GetOperationInfo(uint32_t index)
{
	return _memOperations[index];
}

ArmV3AccessModeVal DummyArmV3Cpu::GetOperationMode(uint32_t index)
{
	return _memAccessMode[index];
}