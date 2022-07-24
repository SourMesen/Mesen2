#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Mapper229 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		WriteRegister(0x8000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectCHRPage(0, addr & 0xFF);
		if(!(addr & 0x1E)) {
			SelectPrgPage2x(0, 0);
		} else {
			SelectPRGPage(0, addr & 0x1F);
			SelectPRGPage(1, addr & 0x1F);
		}
		SetMirroringType(addr & 0x20 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};