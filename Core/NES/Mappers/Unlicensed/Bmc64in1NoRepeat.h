#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Bmc64in1NoRepeat : public BaseMapper
{
private:
	uint8_t _regs[4] = {};

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		AddRegisterRange(0x5000, 0x5003, MemoryOperation::Write);
	}

	void Reset(bool softReset) override
	{
		BaseMapper::Reset(softReset);

		_regs[0] = 0x80;
		_regs[1] = 0x43;
		_regs[2] = _regs[3] = 0;

		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_regs, 4);
	}

	void UpdateState()
	{
		if(_regs[0] & 0x80) {
			if(_regs[1] & 0x80) {
				SelectPrgPage2x(0, (_regs[1] & 0x1F) << 1);
			} else {
				int bank = ((_regs[1] & 0x1F) << 1) | ((_regs[1] >> 6) & 0x01);
				SelectPrgPage(0, bank);
				SelectPrgPage(1, bank);
			}
		} else {
			SelectPrgPage(1, ((_regs[1] & 0x1F) << 1) | ((_regs[1] >> 6) & 0x01));
		}
		SetMirroringType(_regs[0] & 0x20 ? MirroringType::Horizontal : MirroringType::Vertical);
		SelectChrPage(0, (_regs[2] << 2) | ((_regs[0] >> 1) & 0x03));
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_regs[addr & 0x03] = value;
		} else {
			_regs[3] = value;
		}

		UpdateState();
	}
};