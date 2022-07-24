#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Lh32 : public BaseMapper
{
private:
	uint8_t _prgReg = 0;

protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0x6000; }

	void InitMapper() override
	{
		_prgReg = 0;

		SelectCHRPage(0, 0);
		SelectPRGPage(0, -4);
		SelectPRGPage(1, -3);
		SelectPRGPage(2, 0, PrgMemoryType::WorkRam);
		SelectPRGPage(3, -1);

		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_prgReg);

		if(!s.IsSaving()) {
			UpdateState();
		}
	}

	void UpdateState()
	{
		SetCpuMemoryMapping(0x6000, 0x7FFF, _prgReg, PrgMemoryType::PrgRom);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_prgReg = value;
		UpdateState();
	}
};