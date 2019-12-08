#pragma once
#include "stdafx.h"
#include "IMemoryHandler.h"
#include "DebugTypes.h"

class Sa1IRamHandler : public IMemoryHandler
{
private:
	uint8_t * _ram;

public:
	Sa1IRamHandler(uint8_t *ram)
	{
		_ram = ram;
		_memoryType = SnesMemoryType::Sa1InternalRam;
	}

	uint8_t Read(uint32_t addr) override
	{
		if(addr & 0x800) {
			return 0;
		} else {
			return _ram[addr & 0x7FF];
		}
	}

	uint8_t Peek(uint32_t addr) override
	{
		return Read(addr);
	}

	void PeekBlock(uint8_t *output) override
	{
		for(int i = 0; i < 0x1000; i++) {
			output[i] = Read(i);
		}
	}

	void Write(uint32_t addr, uint8_t value) override
	{
		if(!(addr & 0x800)) {
			_ram[addr & 0x7FF] = value;
		}
	}

	AddressInfo GetAbsoluteAddress(uint32_t addr) override
	{
		AddressInfo info;
		if(addr & 0x800) {
			info.Address = -1;
			info.Type = SnesMemoryType::CpuMemory;
		} else {
			info.Address = (addr & 0x7FF);
			info.Type = _memoryType;
		}
		return info;
	}
};