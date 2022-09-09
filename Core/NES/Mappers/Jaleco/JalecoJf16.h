#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class JalecoJf16 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	bool HasBusConflicts() override { return true; }

	void InitMapper() override
	{
		SelectPrgPage(0, GetPowerOnByte());
		SelectPrgPage(1, -1);

		SelectChrPage(0, GetPowerOnByte());
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPrgPage(0, value & 0x07);
		SelectChrPage(0, (value >> 4) & 0x0F);
		if(_romInfo.SubMapperID == 3) {
			//078: 3 Holy Diver
			SetMirroringType(value & 0x08 ? MirroringType::Vertical : MirroringType::Horizontal);
		} else {
			SetMirroringType(value & 0x08 ? MirroringType::ScreenBOnly : MirroringType::ScreenAOnly);
		}
	}
};