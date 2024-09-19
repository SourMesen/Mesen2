#include "pch.h"
#include "WS/Debugger/DummyWsCpu.h"

#define DUMMYCPU
#define WsCpu DummyWsCpu
#include "WS/WsCpu.cpp"
#undef WsCpu
#undef DUMMYCPU

void DummyWsCpu::SetDummyState(WsCpuState& state)
{
	_memOpCounter = 0;
	_state = state;
	_state.Halted = false;
	_state.Flags.Trap = false;
}

uint32_t DummyWsCpu::GetOperationCount()
{
	return _memOpCounter;
}

void DummyWsCpu::LogMemoryOperation(uint32_t addr, uint16_t value, MemoryOperationType type, MemoryType memType, bool isWordAccess)
{
	if(_memOpCounter < 32) {
		_memOperations[_memOpCounter] = {
			addr,
			(int32_t)value,
			type,
			memType
		};
		_memWordAccess[_memOpCounter] = isWordAccess;
		_memOpCounter++;
	}
}

MemoryOperationInfo DummyWsCpu::GetOperationInfo(uint32_t index)
{
	return _memOperations[index];
}

bool DummyWsCpu::IsWordAccess(uint32_t index)
{
	return _memWordAccess[index];
}