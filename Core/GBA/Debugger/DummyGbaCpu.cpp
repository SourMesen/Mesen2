#include "pch.h"
#include "GBA/Debugger/DummyGbaCpu.h"

#define DUMMYCPU
#define GbaCpu DummyGbaCpu
#include "GBA/GbaCpu.cpp"
#include "GBA/GbaCpu.Arm.cpp"
#include "GBA/GbaCpu.Thumb.cpp"
#undef GbaCpu
#undef DUMMYCPU

void DummyGbaCpu::SetDummyState(GbaCpuState& state)
{
	_memOpCounter = 0;
	_state = state;
}

uint32_t DummyGbaCpu::GetOperationCount()
{
	return _memOpCounter;
}

void DummyGbaCpu::LogMemoryOperation(uint32_t addr, uint32_t value, GbaAccessModeVal mode, MemoryOperationType type)
{
	_memOperations[_memOpCounter] = {
		addr,
		(int32_t)value,
		type,
		MemoryType::GbaMemory
	};
	_memAccessMode[_memOpCounter] = mode;
	_memOpCounter++;
}

MemoryOperationInfo DummyGbaCpu::GetOperationInfo(uint32_t index)
{
	return _memOperations[index];
}

GbaAccessModeVal DummyGbaCpu::GetOperationMode(uint32_t index)
{
	return _memAccessMode[index];
}