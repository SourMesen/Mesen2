#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"

class IMemoryHandler
{
protected:
	MemoryType _memoryType;

public:
	IMemoryHandler(MemoryType memType)
	{
		_memoryType = memType;
	}

	virtual ~IMemoryHandler() {}

	virtual uint8_t Read(uint32_t addr) = 0;
	virtual uint8_t Peek(uint32_t addr) = 0;
	virtual void PeekBlock(uint32_t addr, uint8_t *output) = 0;
	virtual void Write(uint32_t addr, uint8_t value) = 0;

	__forceinline MemoryType GetMemoryType()
	{
		return _memoryType;
	}

	virtual AddressInfo GetAbsoluteAddress(uint32_t address) = 0;
};