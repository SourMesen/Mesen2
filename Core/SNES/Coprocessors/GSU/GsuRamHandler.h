#pragma once
#include "pch.h"
#include "SNES/IMemoryHandler.h"
#include "SNES/Coprocessors/GSU/GsuTypes.h"
#include "Shared/MemoryType.h"

class GsuRamHandler : public IMemoryHandler
{
private:
	GsuState *_state;
	IMemoryHandler *_handler;

public:
	GsuRamHandler(GsuState &state, IMemoryHandler *handler) : IMemoryHandler(MemoryType::GsuWorkRam)
	{
		_handler = handler;
		_state = &state;
	}

	uint8_t Read(uint32_t addr) override
	{
		if(!_state->SFR.Running || !_state->GsuRamAccess) {
			return _handler->Read(addr);
		}

		//TODO: open bus
		return 0;
	}

	uint8_t Peek(uint32_t addr) override
	{
		return Read(addr);
	}

	void PeekBlock(uint32_t addr, uint8_t *output) override
	{
		for(int i = 0; i < 0x1000; i++) {
			output[i] = Read(i);
		}
	}

	void Write(uint32_t addr, uint8_t value) override
	{
		if(!_state->SFR.Running || !_state->GsuRamAccess) {
			_handler->Write(addr, value);
		}
	}

	AddressInfo GetAbsoluteAddress(uint32_t address) override
	{
		if(!_state->SFR.Running || !_state->GsuRamAccess) {
			return _handler->GetAbsoluteAddress(address);
		} else {
			return { -1, MemoryType::None };
		}
	}
};