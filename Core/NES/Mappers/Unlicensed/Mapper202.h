#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper202 : public BaseMapper
{
private:
	bool _prgMode1 = false;

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectPrgPage(1, 0);
		SelectChrPage(0, 0);
	}

	virtual void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_prgMode1);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_prgMode1 = (addr & 0x09) == 0x09;
		
		SelectChrPage(0, (addr >> 1) & 0x07);
		if(_prgMode1) {
			SelectPrgPage(0, (addr >> 1) & 0x07);
			SelectPrgPage(1, ((addr >> 1) & 0x07) + 1);
		} else {
			SelectPrgPage(0, (addr >> 1) & 0x07);
			SelectPrgPage(1, (addr >> 1) & 0x07);
		}
		
		SetMirroringType(addr & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};