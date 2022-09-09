#pragma once

#include "pch.h"
#include "NES/Mappers/Namco/Namco108_88.h"

class Namco108_154 : public Namco108_88
{
protected:
	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SetMirroringType((value & 0x40) == 0x40 ? MirroringType::ScreenBOnly : MirroringType::ScreenAOnly);
		Namco108_88::WriteRegister(addr, value);
	}
};