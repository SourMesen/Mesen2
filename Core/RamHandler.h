#pragma once
#include "stdafx.h"
#include "IMemoryHandler.h"
#include "DebugTypes.h"

class RamHandler : public IMemoryHandler
{
private:
	uint8_t * _ram;
	uint32_t _offset;
	SnesMemoryType _memoryType;

public:
	RamHandler(uint8_t *ram, uint32_t offset, SnesMemoryType memoryType)
	{
		_ram = ram + offset;
		_offset = offset;
		_memoryType = memoryType;
	}

	uint8_t Read(uint32_t addr) override
	{
		return _ram[addr & 0xFFF];
	}

	uint8_t Peek(uint32_t addr) override
	{
		return _ram[addr & 0xFFF];
	}

	void Write(uint32_t addr, uint8_t value) override
	{
		_ram[addr & 0xFFF] = value;
	}

	AddressInfo GetAbsoluteAddress(uint32_t address) override
	{
		AddressInfo info;
		info.Address = _offset + (address & 0xFFF);
		info.Type = _memoryType;
		return info;
	}
};