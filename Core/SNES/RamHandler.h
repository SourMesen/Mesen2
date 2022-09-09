#pragma once
#include "pch.h"
#include "SNES/IMemoryHandler.h"
#include "Debugger/DebugTypes.h"

class RamHandler : public IMemoryHandler
{
private:
	uint8_t * _ram;
	uint32_t _mask;

protected:
	uint32_t _offset;

public:
	RamHandler(uint8_t *ram, uint32_t offset, uint32_t size, MemoryType memoryType) : IMemoryHandler(memoryType)
	{
		_ram = ram + offset;
		_offset = offset;

		if(size - offset < 0x1000) {
			_mask = size - offset - 1;
		} else {
			_mask = 0xFFF;
		}
		_memoryType = memoryType;
	}

	uint8_t Read(uint32_t addr) override
	{
		return _ram[addr & _mask];
	}

	uint8_t Peek(uint32_t addr) override
	{
		return _ram[addr & _mask];
	}

	void PeekBlock(uint32_t addr, uint8_t *output) override
	{
		if(_mask != 0xFFF) {
			for(int i = 0; i < 0x1000; i++) {
				output[i] = _ram[i & _mask];
			}
		} else {
			memcpy(output, _ram, 0x1000);
		}
	}

	void Write(uint32_t addr, uint8_t value) override
	{
		_ram[addr & _mask] = value;
	}

	AddressInfo GetAbsoluteAddress(uint32_t address) override
	{
		AddressInfo info;
		info.Address = _offset + (address & _mask);
		info.Type = _memoryType;
		return info;
	}
};