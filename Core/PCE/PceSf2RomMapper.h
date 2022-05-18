#pragma once
#include "stdafx.h"
#include "PCE/IPceMapper.h"

class PceConsole;

class PceSf2RomMapper : public IPceMapper
{
private:
	uint8_t _selectedBank = 0;
	PceConsole* _console = nullptr;

public:
	PceSf2RomMapper(PceConsole* console);
	void Write(uint8_t bank, uint16_t addr, uint8_t value) override;
};
