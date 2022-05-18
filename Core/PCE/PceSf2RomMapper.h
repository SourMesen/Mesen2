#pragma once
#include "stdafx.h"
#include "PCE/IPceMapper.h"

class PceMemoryManager;

class PceSf2RomMapper : public IPceMapper
{
private:
	uint8_t _selectedBank = 0;
	PceMemoryManager* _memoryManager = nullptr;

public:
	PceSf2RomMapper(PceMemoryManager* memoryManager);
	void Write(uint8_t bank, uint16_t addr, uint8_t value) override;
};
