#pragma once
#include "stdafx.h"
#include "IMemoryHandler.h"

class RamHandler : public IMemoryHandler
{
private:
	uint8_t * _ram;

public:
	RamHandler(uint8_t *ram)
	{
		_ram = ram;
	}

	uint8_t Read(uint32_t addr) override
	{
		return _ram[addr & 0xFFF];
	}

	void Write(uint32_t addr, uint8_t value) override
	{
		_ram[addr & 0xFFF] = value;
	}
};