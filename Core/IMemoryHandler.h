#pragma once
#include "stdafx.h"
#include "DebugTypes.h"

class IMemoryHandler
{
protected:
	SnesMemoryType _memoryType;

public:
	IMemoryHandler(SnesMemoryType memType)
	{
		_memoryType = memType;
	}

	virtual ~IMemoryHandler() {}

	virtual uint8_t Read(uint32_t addr) = 0;
	virtual uint8_t Peek(uint32_t addr) = 0;
	virtual void PeekBlock(uint32_t addr, uint8_t *output) = 0;
	virtual void Write(uint32_t addr, uint8_t value) = 0;

	__forceinline SnesMemoryType GetMemoryType()
	{
		return _memoryType;
	}

	virtual AddressInfo GetAbsoluteAddress(uint32_t address) = 0;
};