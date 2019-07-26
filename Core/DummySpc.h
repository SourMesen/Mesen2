#pragma once

#include "stdafx.h"

#define DUMMYSPC
#define Spc DummySpc
#include "Spc.h"
#include "Spc.cpp"
#include "Spc.Instructions.cpp"
#undef Spc
#undef DUMMYSPC

#include "Spc.h"

DummySpc::DummySpc(uint8_t *spcRam, SpcState &state)
{
	_ram = spcRam;

	_opCode = 0;
	_opStep = SpcOpStep::ReadOpCode;
	_opSubStep = 0;
	_tmp1 = 0;
	_tmp2 = 0;
	_tmp3 = 0;
	_operandA = 0;
	_operandB = 0;

	_state = state;
	_writeCounter = 0;
	_readCounter = 0;
}

DummySpc::~DummySpc()
{
	_ram = nullptr;
}

void DummySpc::Step()
{
	do {
		ProcessCycle();
	} while(_opStep != SpcOpStep::ReadOpCode);
}

uint32_t DummySpc::GetWriteCount()
{
	return _writeCounter;
}

uint32_t DummySpc::GetReadCount()
{
	return _readCounter;
}

void DummySpc::LogRead(uint32_t addr, uint8_t value)
{
	_readAddresses[_readCounter] = addr;
	_readValue[_readCounter] = value;
	_readCounter++;
}

void DummySpc::LogWrite(uint32_t addr, uint8_t value)
{
	_writeAddresses[_writeCounter] = addr;
	_writeValue[_writeCounter] = value;
	_writeCounter++;
}

void DummySpc::GetWriteInfo(uint32_t index, uint32_t &addr, uint8_t &value)
{
	addr = _writeAddresses[index];
	value = _writeValue[index];
}

void DummySpc::GetReadInfo(uint32_t index, uint32_t &addr, uint8_t &value)
{
	addr = _readAddresses[index];
	value = _readValue[index];
}