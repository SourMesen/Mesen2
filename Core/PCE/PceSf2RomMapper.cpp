#include "stdafx.h"
#include "PCE/PceSf2RomMapper.h"
#include "PCE/PceMemoryManager.h"

PceSf2RomMapper::PceSf2RomMapper(PceMemoryManager* memoryManager)
{
	_memoryManager = memoryManager;
}

void PceSf2RomMapper::Write(uint16_t addr, uint8_t value)
{
	if((addr & 0x1FFC) == 0x1FF0) {
		_selectedBank = addr & 0x03;

		uint32_t bankOffsets[8] = {
			0x00, 0x10, 0x20, 0x30,
			0x40u + (_selectedBank * 0x40),
			0x50u + (_selectedBank * 0x40),
			0x60u + (_selectedBank * 0x40),
			0x70u + (_selectedBank * 0x40)
		};

		_memoryManager->UpdateMappings(bankOffsets);
	}
}