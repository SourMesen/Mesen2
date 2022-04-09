#pragma once
#include "stdafx.h"

class PceMemoryManager;

class PceSf2RomMapper
{
private:
	uint8_t _selectedBank = 0;
	PceMemoryManager* _memoryManager = nullptr;

public:
	PceSf2RomMapper(PceMemoryManager* memoryManager);
	void Write(uint16_t addr, uint8_t value);
};
