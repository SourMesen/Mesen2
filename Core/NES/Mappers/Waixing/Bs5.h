#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Bs5 : public BaseMapper
{
protected:
	uint32_t GetDipSwitchCount() override { return 2; }
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x800; }

	void InitMapper() override
	{
		for(int i = 0; i < 4; i++) {
			SelectPrgPage(i, -1);
			SelectChrPage(i, -1);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		int bank = (addr >> 10) & 0x03;
		switch(addr & 0xF000) {
			case 0x8000: 
				SelectChrPage(bank, addr & 0x1F); 
				break;

			case 0xA000:
				if(addr & (1 << (GetDipSwitches() + 4))) {
					SelectPrgPage(bank, addr & 0x0F);
				}
				break;
		}
	}
};