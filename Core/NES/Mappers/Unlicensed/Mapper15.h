#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper15 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectChrPage(0, 0);
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
				SelectPrgPage(0, bank  ^ subBank);
				SelectPrgPage(1, (bank + 1) ^ subBank);
				SelectPrgPage(2, (bank + 2) ^ subBank);
				SelectPrgPage(3, (bank + 3) ^ subBank);
				break;

			case 1:
			case 3:
				bank |= subBank;
				SelectPrgPage(0, bank);
				SelectPrgPage(1, bank + 1);
				bank = ((mode == 3) ? bank : (bank | 0x0E)) | subBank;
				SelectPrgPage(2, bank + 0);
				SelectPrgPage(3, bank + 1);
				break;

			case 2:
				bank |= subBank;
				SelectPrgPage(0, bank);
				SelectPrgPage(1, bank);
				SelectPrgPage(2, bank);
				SelectPrgPage(3, bank);
				break;
		}
	}
};
