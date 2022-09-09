#include "pch.h"
#include "NES/NesCpu.h"
#include "NES/Debugger/DummyNesCpu.h"

#define DUMMYCPU
#define NesCpu DummyNesCpu
#include "NES/NesCpu.cpp"
#undef NesCpu
#undef DUMMYCPU

void DummyNesCpu::SetDummyState(NesCpu* c)
{
	_memOpCounter = 0;

	_state = c->_state;

	_operand = c->_operand;
	_spriteDmaTransfer = c->_spriteDmaTransfer;
	_needHalt = c->_needHalt;
	_dmcDmaRunning = c->_dmcDmaRunning;
	_cpuWrite = c->_cpuWrite;
	_needDummyRead = c->_needDummyRead;
	_needHalt = c->_needHalt;
	_spriteDmaOffset = c->_spriteDmaOffset;
	_irqMask = c->_irqMask;
	_prevRunIrq = c->_prevRunIrq;
	_runIrq = c->_runIrq;
}

uint32_t DummyNesCpu::GetOperationCount()
{
	return _memOpCounter;
}

void DummyNesCpu::LogMemoryOperation(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	_memOperations[_memOpCounter] = {
		addr,
		(int32_t)value,
		type,
		MemoryType::NesMemory
	};
	_memOpCounter++;
}

MemoryOperationInfo DummyNesCpu::GetOperationInfo(uint32_t index)
{
	return _memOperations[index];
}