#pragma once
#include "pch.h"
#include "NES/INesMemoryHandler.h"
#include "NES/NesConsole.h"
#include "NES/NesPpu.h"

class Mmc5MemoryHandler : public INesMemoryHandler
{
	NesConsole* _console = nullptr;
	uint8_t _ppuRegs[8] = {};

public:
	Mmc5MemoryHandler(NesConsole* console)
	{
		_console = console;
	}

	uint8_t GetReg(uint16_t addr)
	{
		return _ppuRegs[addr & 0x07];
	}

	void GetMemoryRanges(MemoryRanges& ranges) override {}
	uint8_t ReadRam(uint16_t addr) override { return 0; }

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		_console->GetPpu()->WriteRam(addr, value);
		_ppuRegs[addr & 0x07] = value;
	}
};
