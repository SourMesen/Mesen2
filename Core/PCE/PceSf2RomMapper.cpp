#include "pch.h"
#include "PCE/PceSf2RomMapper.h"
#include "PCE/PceConsole.h"
#include "PCE/PceMemoryManager.h"
#include "Utilities/Serializer.h"

PceSf2RomMapper::PceSf2RomMapper(PceConsole* console)
{
	_console = console;
	_mappedBanks[0] = true;
}

void PceSf2RomMapper::Write(uint8_t bank, uint16_t addr, uint8_t value)
{
	if((addr & 0x1FF0) == 0x1FF0) {
		_selectedBank = addr & 0x0F;
		UpdateMappings();
	}
}

void PceSf2RomMapper::UpdateMappings()
{
	uint32_t bankOffsets[8] = {
		0x00, 0x10, 0x20, 0x30,
		0x40u + (_selectedBank * 0x40),
		0x50u + (_selectedBank * 0x40),
		0x60u + (_selectedBank * 0x40),
		0x70u + (_selectedBank * 0x40)
	};

	_console->GetMemoryManager()->UpdateMappings(bankOffsets);
}

void PceSf2RomMapper::Serialize(Serializer& s)
{
	SV(_selectedBank);
	if(!s.IsSaving()) {
		UpdateMappings();
	}
}
