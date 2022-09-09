#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Ghostbusters63in1 : public BaseMapper
{
private:
	uint8_t _regs[2] = {};

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
	}
	
	void Reset(bool softReset) override
	{
		_regs[0] = _regs[1] = 0;
		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_regs, 2);
		
		if(!s.IsSaving()) {
			UpdateState();
		}
	}

	void UpdateState()
	{
		uint8_t chip = (_regs[1] << 5 & 0x20) << (_regs[0] >> 7);
		if(chip < (_regs[0] >> 7)) {
			RemoveCpuMemoryMapping(0x8000, 0xFFFF);
		} else {
			SelectPrgPage(0, chip | (_regs[0] & 0x1E) | (_regs[0] >> 5 & _regs[0]));
			SelectPrgPage(1, chip | (_regs[0] & 0x1F) | (~_regs[0] >> 5 & 0x01));
		}

		SelectChrPage(0, 0);
		SetMirroringType(_regs[0] & 0x40 ? MirroringType::Vertical : MirroringType::Horizontal);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_regs[addr & 0x01] = value;
		UpdateState();
	}
};