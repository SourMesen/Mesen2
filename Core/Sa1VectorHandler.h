#pragma once
#include "stdafx.h"
#include "IMemoryHandler.h"
#include "Sa1Cpu.h"
#include "Sa1Types.h"

class Sa1VectorHandler : public IMemoryHandler
{
private:
	IMemoryHandler * _handler;
	Sa1State* _state;
	bool _forSnesCpu;

public:
	Sa1VectorHandler(IMemoryHandler* handler, Sa1State* state, bool forSnesCpu)
	{
		_handler = handler;
		_state = state;
		_forSnesCpu = forSnesCpu;
	}

	uint8_t Read(uint32_t addr) override
	{
		if(addr >= Sa1Cpu::NmiVector && addr <= Sa1Cpu::ResetVector + 1) {
			//Override the regular handlers
			if(_forSnesCpu) {
				if(_state->UseCpuNmiVector) {
					if(addr == Sa1Cpu::NmiVector) {
						return (uint8_t)_state->CpuNmiVector;
					} else if(addr == Sa1Cpu::NmiVector + 1) {
						return (uint8_t)(_state->CpuNmiVector >> 8);
					}
				}
				if(_state->UseCpuIrqVector) {
					if(addr == Sa1Cpu::IrqVector) {
						return (uint8_t)_state->CpuIrqVector;
					} else if(addr == Sa1Cpu::IrqVector + 1) {
						return (uint8_t)(_state->CpuIrqVector >> 8);
					}
				}
			} else {
				switch(addr) {
					case Sa1Cpu::NmiVector: return (uint8_t)_state->Sa1NmiVector;
					case Sa1Cpu::NmiVector + 1: return (uint8_t)(_state->Sa1NmiVector >> 8);
					case Sa1Cpu::ResetVector: return (uint8_t)_state->Sa1ResetVector;
					case Sa1Cpu::ResetVector + 1: return (uint8_t)(_state->Sa1ResetVector >> 8);
					case Sa1Cpu::IrqVector: return (uint8_t)_state->Sa1IrqVector;
					case Sa1Cpu::IrqVector + 1: return (uint8_t)(_state->Sa1IrqVector >> 8);
				}
			}
		}

		return _handler->Read(addr);
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