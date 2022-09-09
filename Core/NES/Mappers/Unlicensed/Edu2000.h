#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Edu2000 : public BaseMapper
{
private:
	uint8_t _reg = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint32_t GetWorkRamSize() override { return 0x8000; }
	uint32_t GetWorkRamPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_reg = 0;
		UpdatePrg();
		SelectChrPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_reg);
		if(!s.IsSaving()) {
			UpdatePrg();
		}
	}

	void UpdatePrg()
	{
		SelectPrgPage(0, _reg & 0x1F);
		SetCpuMemoryMapping(0x6000, 0x7FFF, (_reg >> 6) & 0x03, PrgMemoryType::WorkRam);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_reg = value;
		UpdatePrg();
	}
};