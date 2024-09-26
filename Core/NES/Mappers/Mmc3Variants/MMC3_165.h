#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_165 : public MMC3
{
private:
	bool _chrLatch[2] = { false, false };
	bool _needUpdate = false;

protected:
	uint16_t GetChrPageSize() override { return 0x1000; }
	uint32_t GetChrRamSize() override { return 0x1000; }
	uint16_t GetChrRamPageSize() override { return 0x1000; }	
	bool EnableVramAddressHook() override { return true; }

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_chrLatch[0]);
		SV(_chrLatch[1]);
		SV(_needUpdate);
	}

	void UpdateChrMapping() override
	{
		uint16_t page;
		
		for(int i = 0; i < 2; i++) {
			page = _registers[i == 0 ? (_chrLatch[0] ? 1 : 0) : (_chrLatch[1] ? 4 : 2)];
			if(page == 0) {
				SelectChrPage(i, 0, ChrMemoryType::ChrRam);
			} else {
				SelectChrPage(i, page >> 2, ChrMemoryType::ChrRom);
			}
		}

		_needUpdate = false;
	}

	void NotifyVramAddressChange(uint16_t addr) override
	{
		if(_needUpdate) {
			UpdateChrMapping();
		}

		//MMC2 style latch
		switch(addr & 0x2FF8) {
			case 0xFD0: case 0xFE8:
				_chrLatch[(addr >> 12) & 0x01] = ((addr & 0x08) == 0x08);
				_needUpdate = true;
				break;
		}
	}
};