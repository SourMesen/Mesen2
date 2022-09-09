#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class ColorDreams46 : public BaseMapper
{
private:
	uint8_t _regs[2] = {};

protected:
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		WriteRegister(0x6000, 0);
		WriteRegister(0x8000, 0);
	}

	void Reset(bool softReset) override
	{
		WriteRegister(0x6000, 0);
		WriteRegister(0x8000, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_regs, 2);
	}

	void UpdateState()
	{
		SelectPrgPage(0, ((_regs[0] & 0x0F) << 1) | (_regs[1] & 0x01));
		SelectChrPage(0, ((_regs[0] & 0xF0) >> 1) | ((_regs[1] & 0x70) >> 4));
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_regs[0] = value;
		} else {
			_regs[1] = value;
		}
		UpdateState();
	}
};
