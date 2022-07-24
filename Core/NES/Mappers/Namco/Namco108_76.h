#pragma once
#include "stdafx.h"
#include "NES/Mappers/Namco/Namco108.h"

class Namco108_76 : public Namco108
{
	uint16_t GetCHRPageSize() override { return 0x0800; }

protected:
	virtual void UpdateChrMapping() override
	{
		SelectCHRPage(0, _registers[2]);
		SelectCHRPage(1, _registers[3]);
		SelectCHRPage(2, _registers[4]);
		SelectCHRPage(3, _registers[5]);
	}
};