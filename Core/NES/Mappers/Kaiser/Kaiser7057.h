#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Kaiser7057 : public BaseMapper
{
private:
	uint8_t _regs[8] = {};

protected:
	uint16_t GetPrgPageSize() override { return 0x800; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		memset(_regs, 0, sizeof(_regs));
		SelectChrPage(0, 0);
		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_regs, 8);

		if(!s.IsSaving()) {
			UpdateState();
		}
	}

	void UpdateState()
	{
		for(int i = 0; i < 4; i++) {
			SetCpuMemoryMapping(0x6000 + 0x800 * i, 0x67FF + 0x800 * i, _regs[4 + i], PrgMemoryType::PrgRom);
			SelectPrgPage(i, _regs[i]);
		}
		SelectPrgPage4x(1, 0x34);
		SelectPrgPage4x(2, 0x38);
		SelectPrgPage4x(3, 0x3C);
	}

	void UpdatePrgReg(int index, uint8_t value, bool low)
	{
		if(low) {
			_regs[index] = (_regs[index] & 0xF0) | (value & 0x0F);
		} else {
			_regs[index] = (_regs[index] & 0x0F) | ((value << 4) & 0xF0);
		}
		UpdateState();
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		bool low = (addr & 0x01) == 0x00;
		switch(addr & 0xF002) {
			case 0x8000: case 0x8002: case 0x9000: case 0x9002:
				SetMirroringType(value & 0x01 ? MirroringType::Vertical : MirroringType::Horizontal);
				break;

			case 0xB000: UpdatePrgReg(0, value, low); break;
			case 0xB002: UpdatePrgReg(1, value, low); break;
			case 0xC000: UpdatePrgReg(2, value, low); break;
			case 0xC002: UpdatePrgReg(3, value, low); break;
			case 0xD000: UpdatePrgReg(4, value, low); break;
			case 0xD002: UpdatePrgReg(5, value, low); break;
			case 0xE000: UpdatePrgReg(6, value, low); break;
			case 0xE002: UpdatePrgReg(7, value, low); break;
		}
	}
};