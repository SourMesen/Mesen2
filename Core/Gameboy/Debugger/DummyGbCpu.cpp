#include "pch.h"
#include "Gameboy/Debugger/DummyGbCpu.h"

#define DUMMYCPU
#define GbCpu DummyGbCpu
#include "Gameboy/GbCpu.cpp"
#undef GbCpu
#undef DUMMYCPU

void DummyGbCpu::SetDummyState(GbCpuState& state)
{
	_memOpCounter = 0;
	_state = state;
	_state.HaltCounter = 0;
	_state.HaltBug = false;
}

uint32_t DummyGbCpu::GetOperationCount()
{
	return _memOpCounter;
}

void DummyGbCpu::LogMemoryOperation(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	_memOperations[_memOpCounter] = {
		addr,
		(int32_t)value,
		type,
		MemoryType::GameboyMemory
	};
	_memOpCounter++;
}

MemoryOperationInfo DummyGbCpu::GetOperationInfo(uint32_t index)
{
	return _memOperations[index];
}