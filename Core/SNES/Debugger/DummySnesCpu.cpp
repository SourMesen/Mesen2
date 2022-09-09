#include "pch.h"
#include "SNES/Debugger/DummySnesCpu.h"

#define DUMMYCPU
#define SnesCpu DummySnesCpu
#include "SNES/SnesCpu.cpp"
#undef SnesCpu
#undef DUMMYCPU


DummySnesCpu::DummySnesCpu(SnesConsole* console, CpuType type)
{
	_console = console;
	_memoryMappings = type == CpuType::Snes ? console->GetMemoryManager()->GetMemoryMappings() : console->GetCartridge()->GetSa1()->GetMemoryMappings();
	_dmaController = nullptr;
	_memoryManager = nullptr;
}

uint8_t DummySnesCpu::Read(uint32_t addr, MemoryOperationType type)
{
	uint8_t value = _memoryMappings->Peek(addr);
	LogMemoryOperation(addr, value, type);
	return value;
}

void DummySnesCpu::Write(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	LogMemoryOperation(addr, value, type);
}

void DummySnesCpu::SetDummyState(SnesCpuState& state)
{
	_state = state;
	_state.StopState = SnesCpuStopState::Running;
	_memOpCounter = 0;
}

uint32_t DummySnesCpu::GetOperationCount()
{
	return _memOpCounter;
}

void DummySnesCpu::LogMemoryOperation(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	_memOperations[_memOpCounter] = {
		addr,
		(int32_t)value,
		type,
		MemoryType::SnesMemory
	};
	_memOpCounter++;
}

MemoryOperationInfo DummySnesCpu::GetOperationInfo(uint32_t index)
{
	return _memOperations[index];
}

int32_t DummySnesCpu::GetLastOperand()
{
	return _operand;
}