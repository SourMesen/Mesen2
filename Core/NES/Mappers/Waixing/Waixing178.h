#pragma once
#include "NES/BaseMapper.h"

class Waixing178 : public BaseMapper
{
private:
	uint8_t _regs[4] = {};

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x4800; }
	uint16_t RegisterEndAddress() override { return 0x4FFF; }
	uint32_t GetWorkRamSize() override { return 0x8000; }

	void InitMapper() override
	{
		memset(_regs, 0, sizeof(_regs));
		UpdateState();
		SelectChrPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_regs, 4);
		if(!s.IsSaving()) {
			UpdateState();
		}
	}

	void UpdateState()
	{
		uint16_t sbank = _regs[1] & 0x07;
		uint16_t bbank = _regs[2];
		if(_regs[0] & 0x02) {
			SelectPrgPage(0, (bbank << 3) | sbank);
			if(_regs[0] & 0x04) {
				SelectPrgPage(1, (bbank << 3) | 0x06 | (_regs[1] & 0x01));
			} else {
				SelectPrgPage(1, (bbank << 3) | 0x07);
			}
		} else {
			uint16_t bank = (bbank << 3) | sbank;
			if(_regs[0] & 0x04) {
				SelectPrgPage(0, bank);
				SelectPrgPage(1, bank);
			} else {
				SelectPrgPage2x(0, bank);
			}
		}

		SetCpuMemoryMapping(0x6000, 0x7FFF, _regs[3] & 0x03, PrgMemoryType::WorkRam, MemoryAccessType::ReadWrite);
		SetMirroringType(_regs[0] & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_regs[addr & 0x03] = value;
		UpdateState();
	}
};