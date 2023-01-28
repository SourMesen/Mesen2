#pragma once
#include "NES/BaseMapper.h"

class MagicFloor218 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);

		if(GetMirroringType() == MirroringType::FourScreens) {
			SetMirroringType(_romInfo.Header.Byte6 & 0x01 ? MirroringType::ScreenBOnly : MirroringType::ScreenAOnly);
		}

		uint16_t mask = 0;
		switch(GetMirroringType()) {
			case MirroringType::Vertical: mask = 0x400; break;
			case MirroringType::Horizontal: mask = 0x800; break;
			case MirroringType::ScreenAOnly: mask = 0x1000; break;
			case MirroringType::ScreenBOnly: mask = 0x2000; break;
			case MirroringType::FourScreens: break; //Will never be FourScreens, see InitMapper() above
		}

		for(int i = 0; i < 8; i++) {
			SetPpuMemoryMapping(i*0x400, i*0x400+0x3FF, ((i * 0x400) & mask) ? 1 : 0, ChrMemoryType::NametableRam);
		}
	}
};
