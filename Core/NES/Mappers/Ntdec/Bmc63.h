#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Bmc63 : public BaseMapper
{
private:
	bool _openBus = false;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		WriteRegister(0x8000, 0);
	}

	void Reset(bool softReset) override
	{
		_openBus = false;
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_openBus);
		if(!s.IsSaving() && _openBus) {
			RemoveCpuMemoryMapping(0x8000, 0xBFFF);
		}
	}
	
	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_openBus = ((addr & 0x0300) == 0x0300);

		if(_openBus) {
			RemoveCpuMemoryMapping(0x8000, 0xBFFF);
		} else {
			SelectPrgPage(0, (addr >> 1 & 0x1FC) | ((addr & 0x2) ? 0x0 : (addr >> 1 & 0x2) | 0x0));
			SelectPrgPage(1, (addr >> 1 & 0x1FC) | ((addr & 0x2) ? 0x1 : (addr >> 1 & 0x2) | 0x1));
		}
		SelectPrgPage(2, (addr >> 1 & 0x1FC) | ((addr & 0x2) ? 0x2 : (addr >> 1 & 0x2) | 0x0));
		SelectPrgPage(3, (addr & 0x800) ? ((addr & 0x07C) | ((addr & 0x06) ? 0x03 : 0x01)) : ((addr >> 1 & 0x01FC) | ((addr & 0x02) ? 0x03 : ((addr >> 1 & 0x02) | 0x01))));

		SetMirroringType(addr & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};