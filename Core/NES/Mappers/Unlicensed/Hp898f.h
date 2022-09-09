#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Hp898f : public BaseMapper
{
private:
	uint8_t _regs[2] = {};

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		_regs[0] = _regs[1] = 0;
		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_regs[0]);
		SV(_regs[1]);
	}

	void UpdateState()
	{
		uint8_t prgReg = (_regs[1] >> 3) & 7;
		uint8_t prgMask = (_regs[1] >> 4) & 4;
		SelectChrPage(0, (((_regs[0] >> 4) & 0x07) & ~(((_regs[0] & 0x01) << 2) | (_regs[0] & 0x02))));
		SelectPrgPage(0, prgReg & (~prgMask));
		SelectPrgPage(1, prgReg | prgMask);
		SetMirroringType(_regs[1] & 0x80 ? MirroringType::Vertical : MirroringType::Horizontal);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if((addr & 0x6000) == 0x6000) {
			_regs[(addr & 0x04) >> 2] = value;
			UpdateState();
		}
	}
};