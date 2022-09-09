#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper120 : public BaseMapper
{
private:
	uint8_t _prgReg = 0;

protected:
	uint16_t RegisterStartAddress() override { return 0x41FF; }
	uint16_t RegisterEndAddress() override { return 0x41FF; }
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_prgReg = 0;
		UpdatePrg();
		SelectPrgPage4x(0, 8);
		SelectChrPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_prgReg);
		if(!s.IsSaving()) {
			UpdatePrg();
		}
	}

	void UpdatePrg()
	{
		SetCpuMemoryMapping(0x6000, 0x7FFF, _prgReg, PrgMemoryType::PrgRom);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_prgReg = value;
		UpdatePrg();
	}
};
