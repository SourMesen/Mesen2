#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class IremLrog017 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x0800; }
	uint32_t GetChrRamSize() override { return 0x1800; }
	uint16_t GetChrRamPageSize() override { return 0x0800; }
	bool HasBusConflicts() override { return true; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectChrPage(0, 0);
		SetMirroringType(MirroringType::FourScreens);

		SelectChrPage(1, 0, ChrMemoryType::ChrRam);
		SelectChrPage(2, 1, ChrMemoryType::ChrRam);
		SelectChrPage(3, 2, ChrMemoryType::ChrRam);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPrgPage(0, value & 0x0F);
		SelectChrPage(0, (value >> 4) & 0x0F);
	}
};