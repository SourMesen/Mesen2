#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class VRC1 : public BaseMapper
{
private:
	uint8_t _chrBanks[2] = {};
	
	void UpdateChrBanks()
	{
		SelectCHRPage(0, _chrBanks[0]);
		SelectCHRPage(1, _chrBanks[1]);
	}

protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x1000; }

	void InitMapper() override
	{
		SelectPRGPage(3, -1);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_chrBanks[0]);
		SV(_chrBanks[1]);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		//TODO: Create a setting to enable/disable oversized PRG
		bool allowOversizedPrg = true;
		uint8_t prgMask = allowOversizedPrg ? 0xFF : 0x0F;

		switch(addr & 0xF000) {
			case 0x8000: SelectPRGPage(0, value & prgMask); break;
			
			case 0x9000:
				if(GetMirroringType() != MirroringType::FourScreens) {
					//"The mirroring bit is ignored if the cartridge is wired for 4-screen VRAM, as is typical for Vs. System games using the VRC1."
					SetMirroringType((value & 0x01) == 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
				}
				_chrBanks[0] = (_chrBanks[0] & 0x0F) | ((value & 0x02) << 3);
				_chrBanks[1] = (_chrBanks[1] & 0x0F) | ((value & 0x04) << 2);
				UpdateChrBanks();
				break;

			case 0xA000: SelectPRGPage(1, value & prgMask); break;
			case 0xC000: SelectPRGPage(2, value & prgMask); break;

			case 0xE000:
				_chrBanks[0] = (_chrBanks[0] & 0x10) | (value & 0x0F);
				UpdateChrBanks();
				break;

			case 0xF000:
				_chrBanks[1] = (_chrBanks[1] & 0x10) | (value & 0x0F);
				UpdateChrBanks();
				break;
		}
	}
};