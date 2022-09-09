#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class BmcG146 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
	}

	void Reset(bool softReset) override
	{
		BaseMapper::Reset(softReset);

		WriteRegister(0x8000, 0);
		SelectChrPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr & 0x800) {
			SelectPrgPage(0, (addr & 0x1F) | (addr & ((addr & 0x40) >> 6)));
			SelectPrgPage(1, (addr & 0x18) | 0x07);
		} else {
			if(addr & 0x40) {
				SelectPrgPage(0, addr & 0x1F);
				SelectPrgPage(1, addr & 0x1F);
			} else {
				SelectPrgPage2x(0, addr & 0x1E);
			}
		}
		SetMirroringType(addr & 0x80 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};