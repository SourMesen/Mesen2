#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Mapper200 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectPRGPage(1, 0);
		SelectCHRPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		uint8_t bank = addr & 0x07;
		SelectPRGPage(0, bank);
		SelectPRGPage(1, bank);
		SelectCHRPage(0, bank);

		SetMirroringType(addr & 0x08 ? MirroringType::Vertical : MirroringType::Horizontal);
	}
};
