#pragma once
#include "pch.h"
#include "SNES/IMemoryHandler.h"
#include "SNES/Coprocessors/GSU/GsuTypes.h"
#include "Shared/MemoryType.h"

class GsuRomHandler : public IMemoryHandler
{
private:
	GsuState *_state;
	IMemoryHandler *_romHandler;

public:
	GsuRomHandler(GsuState &state, IMemoryHandler *romHandler) : IMemoryHandler(MemoryType::SnesPrgRom)
	{
		_romHandler = romHandler;
		_state = &state;
	}

	uint8_t Read(uint32_t addr) override
	{
		if(!_state->SFR.Running || !_state->GsuRomAccess) {
			return _romHandler->Read(addr);
		}

		if(addr & 0x01) {
			return 0x01;
		}

		switch(addr & 0x0E) {
			default:
			case 2: case 6: case 8: case 0x0C: 
				return 0;

			case 4: return 0x04;
			case 0x0A: return 0x08;
			case 0x0E: return 0x0C;
		}
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
		//ROM
	}

	AddressInfo GetAbsoluteAddress(uint32_t address) override
	{
		if(!_state->SFR.Running || !_state->GsuRomAccess) {
			return _romHandler->GetAbsoluteAddress(address);
		} else {
			return { -1, MemoryType::None };
		}
	}
};