#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper242 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		Reset(false);
		SelectChrPage(0, 0);
	}

	void Reset(bool softReset) override
	{
		SelectPrgPage(0, 0);
		SetMirroringType(MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SetMirroringType(addr & 0x02 ? MirroringType::Horizontal : MirroringType::Vertical);
		SelectPrgPage(0, (addr >> 3) & 0x0F);
	}
};