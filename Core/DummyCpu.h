#pragma once

#include "stdafx.h"

#define DUMMYCPU
#define Cpu DummyCpu
#include "Cpu.h"
#include "Cpu.cpp"
#undef Cpu
#undef DUMMYCPU

void DummyCpu::SetDummyState(CpuState &state)
{
	_state = state;
	_state.StopState = CpuStopState::Running;
	_writeCounter = 0;
	_readCounter = 0;
}

uint32_t DummyCpu::GetWriteCount()
{
	return _writeCounter;
}

uint32_t DummyCpu::GetReadCount()
{
	return _readCounter;
}

void DummyCpu::LogRead(uint32_t addr, uint8_t value)
{
	_readAddresses[_readCounter] = addr;
	_readValue[_readCounter] = value;
	_readCounter++;
}

void DummyCpu::LogWrite(uint32_t addr, uint8_t value)
{
	_writeAddresses[_writeCounter] = addr;
	_writeValue[_writeCounter] = value;
	_writeCounter++;
}

void DummyCpu::GetWriteInfo(uint32_t index, uint32_t &addr, uint8_t &value)
{
	addr = _writeAddresses[index];
	value = _writeValue[index];
}

void DummyCpu::GetReadInfo(uint32_t index, uint32_t &addr, uint8_t &value)
{
	addr = _readAddresses[index];
	value = _readValue[index];
}

int32_t DummyCpu::GetLastOperand()
{
	return _operand;
}