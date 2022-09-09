#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Nina03_06 : public BaseMapper
{
private: 
	bool _multicartMode = false;

protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x4100; }
	uint16_t RegisterEndAddress() override { return 0x5FFF; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectChrPage(0, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if((addr & 0xE100) == 0x4100) {
			if(_multicartMode) {
				//Mapper 113
				SelectPrgPage(0, (value >> 3) & 0x07);
				SelectChrPage(0, (value & 0x07) | ((value >> 3) & 0x08));
				SetMirroringType((value & 0x80) == 0x80 ? MirroringType::Vertical : MirroringType::Horizontal);
			} else {
				SelectPrgPage(0, (value >> 3) & 0x01);
				SelectChrPage(0, value & 0x07);
			}
		}
	}

public:
	Nina03_06(bool multicartMode) : _multicartMode(multicartMode)
	{
	}
};