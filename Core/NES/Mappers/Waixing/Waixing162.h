#pragma once
#include "NES/BaseMapper.h"

class Waixing162 : public BaseMapper
{
private:
	uint8_t _regs[4] = {};

protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x5000; }
	uint16_t RegisterEndAddress() override { return 0x5FFF; }

	void InitMapper() override
	{
		_regs[0] = 3;
		_regs[1] = 0;
		_regs[2] = 0;
		_regs[3] = 7;

		SelectChrPage(0, 0);
		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_regs, 4);
	}

	void UpdateState()
	{
		switch(_regs[3] & 0x5) {
			case 0: SelectPrgPage(0, (_regs[0] & 0x0C) | (_regs[1] & 0x02) | ((_regs[2] & 0x0F) << 4)); break;
			case 1: SelectPrgPage(0, (_regs[0] & 0x0C) | (_regs[2] & 0x0F) << 4); break;
			case 4: SelectPrgPage(0, (_regs[0] & 0x0E) | ((_regs[1] >> 1) & 0x01) | ((_regs[2] & 0x0F) << 4)); break;
			case 5: SelectPrgPage(0, (_regs[0] & 0x0F) | ((_regs[2] & 0x0F) << 4)); break;
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_regs[(addr >> 8) & 0x03] = value;
		UpdateState();
	}
};