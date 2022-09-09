#include "pch.h"
#include "PCE/Debugger/DummyPceCpu.h"

#define DUMMYCPU
#define PceCpu DummyPceCpu
#include "PCE/PceCpu.cpp"
#include "PCE/PceCpu.Instructions.cpp"
#undef PceCpu
#undef DUMMYCPU

DummyPceCpu::DummyPceCpu(Emulator* emu, PceMemoryManager* memoryManager)
{
	_emu = emu;
	_memoryManager = memoryManager;
}

uint8_t DummyPceCpu::MemoryRead(uint16_t addr, MemoryOperationType type)
{
	uint8_t value = _memoryManager->DebugRead(addr);
	LogMemoryOperation(addr, value, type);
	return value;
}

void DummyPceCpu::MemoryWrite(uint16_t addr, uint8_t value, MemoryOperationType type)
{
	LogMemoryOperation(addr, value, type);
}

void DummyPceCpu::SetDummyState(PceCpuState& state)
{
	_state = state;
	_memOpCounter = 0;
}

uint32_t DummyPceCpu::GetOperationCount()
{
	return _memOpCounter;
}

void DummyPceCpu::LogMemoryOperation(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	if(_memOpCounter >= 10) {
		//Can happen for block transfer operations
		return;
	}

	_memOperations[_memOpCounter] = {
		addr,
		(int32_t)value,
		type,
		MemoryType::PceMemory
	};
	_memOpCounter++;
}

MemoryOperationInfo DummyPceCpu::GetOperationInfo(uint32_t index)
{
	return _memOperations[index];
}
