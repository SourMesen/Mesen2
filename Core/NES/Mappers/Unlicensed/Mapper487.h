#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper487 : public BaseMapper
{
private:
	uint8_t _regs[2] = {};

protected:
	uint16_t RegisterStartAddress() override { return 0x4100; }
	uint16_t RegisterEndAddress() override { return 0x5FFF; }
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		AddRegisterRange(0x8000, 0xFFFF, MemoryOperation::Write);
		UpdateState();
	}

	void Reset(bool softReset) override
	{
		_regs[0] = 0;
		_regs[1] = 0;
		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_regs, 2);
	}

	void UpdateState()
	{
		uint8_t prg = _regs[1] & 0x1E;
		uint8_t chr = (
			((_regs[1] & 0x1E) << 2) |
			(_regs[0] & 0x03)
		);

		if(_regs[1] & 0x40) {
			//64kb banks, use inner bank for A15
			prg |= (_regs[0] & 0x08) >> 3;
			chr |= _regs[0] & 0x04;
		} else {
			//32kb banks, use outer bank LSB as A15
			prg |= _regs[1] & 0x01;
			chr |= (_regs[1] & 0x01) << 2;
		}

		if(_regs[1] & 0x20) {
			//Skip first 512kb of PRG/CHR when using 2nd set of prg/chr roms
			prg += 0x10;
			chr += 0x40;
		}

		SelectPrgPage(0, prg);
		SelectChrPage(0, chr);
		
		SetMirroringType(_regs[1] & 0x80 ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x6000) {
			if(addr & 0x100) {
				if(addr & 0x80) {
					_regs[1] = value;
				} else if(!(_regs[1] & 0x20)) {
					//NINA03-style register
					_regs[0] = value & 0x0F;
				}
				UpdateState();
			}
		} else if(_regs[1] & 0x20) {
			//Color dreams-style register
			_regs[0] = ((value & 0x01) << 3) | ((value & 0x70) >> 4);
			UpdateState();
		}
	}
};
