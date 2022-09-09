#pragma once
#include "pch.h"
#include "PCE/IPceMapper.h"
#include "Utilities/ISerializable.h"

class PceConsole;

class PceSf2RomMapper final : public IPceMapper
{
private:
	uint8_t _selectedBank = 0;
	PceConsole* _console = nullptr;

	void UpdateMappings();

public:
	PceSf2RomMapper(PceConsole* console);
	void Write(uint8_t bank, uint16_t addr, uint8_t value) override;

	void Serialize(Serializer& s) override;
};
