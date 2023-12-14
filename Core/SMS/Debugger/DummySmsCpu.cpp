#include "pch.h"
#include "SMS/Debugger/DummySmsCpu.h"

#define DUMMYCPU
#define SmsCpu DummySmsCpu
#include "SMS/SmsCpu.cpp"
#undef SmsCpu
#undef DUMMYCPU

void DummySmsCpu::SetDummyState(SmsCpuState& state)
{
	_memOpCounter = 0;
	_state = state;
	_state.Halted = false;
	_state.NmiLevel = false;
	_state.NmiPending = false;
	_state.ActiveIrqs = 0;
}

uint32_t DummySmsCpu::GetOperationCount()
{
	return _memOpCounter;
}

void DummySmsCpu::LogMemoryOperation(uint32_t addr, uint8_t value, MemoryOperationType type, MemoryType memType)
{
	_memOperations[_memOpCounter] = {
		addr,
		(int32_t)value,
		type,
		memType
	};
	_memOpCounter++;
}

MemoryOperationInfo DummySmsCpu::GetOperationInfo(uint32_t index)
{
	return _memOperations[index];
}