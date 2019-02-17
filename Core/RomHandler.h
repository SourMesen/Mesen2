#pragma once
#include "stdafx.h"
#include "IMemoryHandler.h"

class RomHandler : public IMemoryHandler
{
private:
	uint8_t * _rom;

public:
	RomHandler(uint8_t *rom)
	{
		_rom = rom;
	}

	uint8_t Read(uint32_t addr) override
	{
		return _rom[addr & 0xFFF];
	}

	void Write(uint32_t addr, uint8_t value) override
	{
	}
};