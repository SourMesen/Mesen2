#pragma once

#include "stdafx.h"
#include "Debugger/DebugTypes.h"
#include "SNES/MemoryMappings.h"
#include "SNES/BaseCartridge.h"
#include "SNES/Coprocessors/SA1/Sa1.h"

#define DUMMYCPU
#define SnesCpu DummySnesCpu
#include "SNES/SnesCpu.h"
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
	LogRead(addr, value);
	return value;
}

void DummySnesCpu::Write(uint32_t addr, uint8_t value, MemoryOperationType type)
{
	LogWrite(addr, value);
}

void DummySnesCpu::SetDummyState(SnesCpuState &state)
{
	_state = state;
	_state.StopState = SnesCpuStopState::Running;
	_writeCounter = 0;
	_readCounter = 0;
}

uint32_t DummySnesCpu::GetWriteCount()
{
	return _writeCounter;
}

uint32_t DummySnesCpu::GetReadCount()
{
	return _readCounter;
}

void DummySnesCpu::LogRead(uint32_t addr, uint8_t value)
{
	_readAddresses[_readCounter] = addr;
	_readValue[_readCounter] = value;
	_readCounter++;
}

void DummySnesCpu::LogWrite(uint32_t addr, uint8_t value)
{
	_writeAddresses[_writeCounter] = addr;
	_writeValue[_writeCounter] = value;
	_writeCounter++;
}

void DummySnesCpu::GetWriteInfo(uint32_t index, uint32_t &addr, uint8_t &value)
{
	addr = _writeAddresses[index];
	value = _writeValue[index];
}

void DummySnesCpu::GetReadInfo(uint32_t index, uint32_t &addr, uint8_t &value)
{
	addr = _readAddresses[index];
	value = _readValue[index];
}

int32_t DummySnesCpu::GetLastOperand()
{
	return _operand;
}