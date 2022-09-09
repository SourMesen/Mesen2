#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Ax5705 : public BaseMapper
{
private:
	uint8_t _chrReg[8] = {};

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x400; }

	void InitMapper() override
	{
		memset(_chrReg, 0, sizeof(_chrReg));

		SelectPrgPage(2, -2);
		SelectPrgPage(3, -1);

		for(int i = 0; i < 8; i++) {
			SelectChrPage(i, _chrReg[i]);
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_chrReg, 8);
	}

	void UpdateChrReg(int index, uint8_t value, bool low)
	{
		if(low) {
			_chrReg[index] = (_chrReg[index] & 0xF0) | (value & 0x0F);
		} else {
			_chrReg[index] = (_chrReg[index] & 0x0F) | ((((value & 0x04) >> 1) | ((value & 0x02) << 1) | (value & 0x09)) << 4);
		}
		SelectChrPage(index, _chrReg[index]);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr >= 0xA008) {
			bool low = (addr & 0x01) == 0x00;
			switch(addr & 0xF00E) {
				case 0xA008: UpdateChrReg(0, value, low); break;
				case 0xA00A: UpdateChrReg(1, value, low); break;
				case 0xC000: UpdateChrReg(2, value, low); break;
				case 0xC002: UpdateChrReg(3, value, low); break;
				case 0xC008: UpdateChrReg(4, value, low); break;
				case 0xC00A: UpdateChrReg(5, value, low); break;
				case 0xE000: UpdateChrReg(6, value, low); break;
				case 0xE002: UpdateChrReg(7, value, low); break;
			}
		} else {
			switch(addr & 0xF00F) {
				case 0x8000: SelectPrgPage(0, ((value & 0x02) << 2) | ((value & 0x08) >> 2) | (value & 0x05)); break; // EPROM dump have mixed PRG and CHR banks, data lines to mapper seems to be mixed
				case 0x8008: SetMirroringType(value & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical); break;
				case 0xA000: SelectPrgPage(1, ((value & 0x02) << 2) | ((value & 0x08) >> 2) | (value & 0x05)); break;
			}
		}
	}
};