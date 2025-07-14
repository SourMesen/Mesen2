#pragma once
#include "pch.h"
#include "SNES/IMemoryHandler.h"
#include "SNES/RamHandler.h"
#include "SNES/Coprocessors/SA1/Sa1Cpu.h"
#include "SNES/Coprocessors/SA1/Sa1Types.h"
#include "SNES/Coprocessors/SA1/Sa1.h"

//Manages BWRAM access from the SNES CPU
//Returns conversion result when char conversion type 1 is enabled
class CpuBwRamHandler : public IMemoryHandler
{
private:
	RamHandler* _handler = nullptr;
	Sa1State* _state = nullptr;
	Sa1* _sa1 = nullptr;

public:
	CpuBwRamHandler(RamHandler* handler, Sa1State* state, Sa1* sa1) : IMemoryHandler(handler->GetMemoryType())
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
		if(_state->Sa1BwWriteEnabled || _state->CpuBwWriteEnabled) {
			_handler->Write(addr, value);
		} else {
			uint32_t size = 256 << (_state->BwWriteProtectedArea > 0x0A ? 0x0A : _state->BwWriteProtectedArea);
			if((addr & 0xE00000) == 0x400000 && (addr & 0x3FFFF) >= size) {
				_handler->Write(addr, value);
			} else if(_handler->GetOffset() + (addr & 0xFFF) >= size) {
				_handler->Write(addr, value);
			}
		}
	}

	AddressInfo GetAbsoluteAddress(uint32_t address) override
	{
		return _handler->GetAbsoluteAddress(address);
	}
};