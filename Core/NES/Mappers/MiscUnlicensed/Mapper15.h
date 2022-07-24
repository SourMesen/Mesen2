#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Mapper15 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectCHRPage(0, 0);
	}

	void Reset(bool softReset) override
	{
		WriteRegister(0x8000, 0);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SetMirroringType(value & 0x40 ? MirroringType::Horizontal : MirroringType::Vertical);
		
		uint8_t subBank = value >> 7;
		uint8_t bank = (value & 0x7F) << 1;
		uint8_t mode = addr & 0x03;
		
		SetPpuMemoryMapping(0, 0x1FFF, 0, ChrMemoryType::Default, (mode == 0 || mode == 3) ? MemoryAccessType::Read : MemoryAccessType::ReadWrite);
		
		switch(mode) {
			case 0:
				SelectPRGPage(0, bank  ^ subBank);
				SelectPRGPage(1, (bank + 1) ^ subBank);
				SelectPRGPage(2, (bank + 2) ^ subBank);
				SelectPRGPage(3, (bank + 3) ^ subBank);
				break;

			case 1:
			case 3:
				bank |= subBank;
				SelectPRGPage(0, bank);
				SelectPRGPage(1, bank + 1);
				bank = ((mode == 3) ? bank : (bank | 0x0E)) | subBank;
				SelectPRGPage(2, bank + 0);
				SelectPRGPage(3, bank + 1);
				break;

			case 2:
				bank |= subBank;
				SelectPRGPage(0, bank);
				SelectPRGPage(1, bank);
				SelectPRGPage(2, bank);
				SelectPRGPage(3, bank);
				break;
		}
	}
};
