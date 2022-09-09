#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_250 : public MMC3
{
protected:
	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		MMC3::WriteRegister((addr & 0xE000) | ((addr & 0x0400) >> 10), addr & 0xFF);
	}
};