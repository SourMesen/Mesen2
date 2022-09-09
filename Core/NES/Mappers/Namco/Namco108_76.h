#pragma once
#include "pch.h"
#include "NES/Mappers/Namco/Namco108.h"

class Namco108_76 : public Namco108
{
	uint16_t GetChrPageSize() override { return 0x0800; }

protected:
	virtual void UpdateChrMapping() override
	{
		SelectChrPage(0, _registers[2]);
		SelectChrPage(1, _registers[3]);
		SelectChrPage(2, _registers[4]);
		SelectChrPage(3, _registers[5]);
	}
};