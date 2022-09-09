#pragma once
#include "pch.h"
#include "NES/INesMemoryHandler.h"

template<size_t Mask>
class InternalRamHandler : public INesMemoryHandler
{
private:
	uint8_t *_internalRam = nullptr;

public:
	void SetInternalRam(uint8_t* internalRam)
	{
		_internalRam = internalRam;
	}

	void GetMemoryRanges(MemoryRanges &ranges) override
	{
		ranges.SetAllowOverride();
		ranges.AddHandler(MemoryOperation::Any, 0, 0x1FFF);
	}

	uint8_t ReadRam(uint16_t addr) override
	{
		return _internalRam[addr & Mask];
	}

	uint8_t PeekRam(uint16_t addr) override
	{
		return ReadRam(addr);
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		_internalRam[addr & Mask] = value;
	}
};
