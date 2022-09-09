#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"

class IMemoryHandler;

class MemoryMappings
{
private:
	IMemoryHandler* _handlers[0x100 * 0x10] = {};

public:
	void RegisterHandler(uint8_t startBank, uint8_t endBank, uint16_t startPage, uint16_t endPage, vector<unique_ptr<IMemoryHandler>>& handlers, uint16_t pageIncrement = 0, uint16_t startPageNumber = 0);
	void RegisterHandler(uint8_t startBank, uint8_t endBank, uint16_t startAddr, uint16_t endAddr, IMemoryHandler* handler);

	IMemoryHandler* GetHandler(uint32_t addr);
	AddressInfo GetAbsoluteAddress(uint32_t addr);
	int GetRelativeAddress(AddressInfo& absAddress, uint8_t startBank = 0);

	uint8_t Peek(uint32_t addr);
	uint16_t PeekWord(uint32_t addr);
	void PeekBlock(uint32_t addr, uint8_t * dest);

	void DebugWrite(uint32_t addr, uint8_t value);
};
