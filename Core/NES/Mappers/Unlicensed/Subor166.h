#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Subor166 : public BaseMapper
{
private:
	uint8_t _regs[4] = {};

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		memset(_regs, 0, sizeof(_regs));
		WriteRegister(0x8000, 0);
		SelectChrPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_regs, 4);
	}
	
	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xE000) {
			case 0x8000: _regs[0] = value & 0x10; break;
			case 0xA000: _regs[1] = value & 0x1C; break;
			case 0xC000: _regs[2] = value & 0x1F; break;
			case 0xE000: _regs[3] = value & 0x1F; break;
		}

		uint8_t outerBank = ((_regs[0] ^ _regs[1]) & 0x10) << 1;
		uint8_t innerBank = _regs[2] ^ _regs[3];
		bool altMode = _romInfo.MapperID == 167;

		if(_regs[1] & 0x08) {
			//32 KiB NROM
			uint8_t bank = (outerBank | innerBank) & 0xFE;
			SelectPrgPage(0, altMode  ? bank + 1 : bank);
			SelectPrgPage(1, altMode ? bank : bank + 1);
		} else if(_regs[1] & 0x04) {
			//512 KiB inverted UNROM(mapper 180)
			SelectPrgPage(0, 0x1F);
			SelectPrgPage(1, outerBank | innerBank);
		} else {
			//512 KiB UNROM
			SelectPrgPage(0, outerBank | innerBank);
			SelectPrgPage(1, altMode ? 0x20 : 0x07);
		}
	}
};