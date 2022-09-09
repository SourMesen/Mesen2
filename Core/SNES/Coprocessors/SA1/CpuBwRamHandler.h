#pragma once
#include "pch.h"
#include "SNES/IMemoryHandler.h"
#include "SNES/Coprocessors/SA1/Sa1Cpu.h"
#include "SNES/Coprocessors/SA1/Sa1Types.h"
#include "SNES/Coprocessors/SA1/Sa1.h"

//Manages BWRAM access from the SNES CPU
//Returns conversion result when char conversion type 1 is enabled
class CpuBwRamHandler : public IMemoryHandler
{
private:
	IMemoryHandler * _handler;
	Sa1State* _state;
	Sa1* _sa1;

public:
	CpuBwRamHandler(IMemoryHandler* handler, Sa1State* state, Sa1* sa1) : IMemoryHandler(handler->GetMemoryType())
	{
		_handler = handler;
		_sa1 = sa1;
		_state = state;
	}

	uint8_t Read(uint32_t addr) override
	{
		if(_state->CharConvDmaActive) {
			return _sa1->ReadCharConvertType1(addr);
		} else {
			return _handler->Read(addr);
		}
	}

	uint8_t Peek(uint32_t addr) override
	{
		return Read(addr);
	}

	void PeekBlock(uint32_t addr, uint8_t *output) override
	{
		_handler->PeekBlock(addr, output);
	}

	void Write(uint32_t addr, uint8_t value) override
	{
		_handler->Write(addr, value);
	}

	AddressInfo GetAbsoluteAddress(uint32_t address) override
	{
		return _handler->GetAbsoluteAddress(address);
	}
};