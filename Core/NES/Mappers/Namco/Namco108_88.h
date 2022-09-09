#pragma once
#include "pch.h"
#include "NES/Mappers/Namco/Namco108.h"

class Namco108_88 : public Namco108
{
protected:
	virtual void UpdateChrMapping() override
	{
		_registers[0] &= 0x3F;
		_registers[1] &= 0x3F;

		_registers[2] |= 0x40;
		_registers[3] |= 0x40;
		_registers[4] |= 0x40;
		_registers[5] |= 0x40;

		Namco108::UpdateChrMapping();
	}
};