#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class T262 : public BaseMapper
{
private:
	bool _locked = false;
	uint8_t _base = 0;
	uint8_t _bank = 0;
	bool _mode = false;

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectPrgPage(1, 7);
		SelectChrPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_locked);
		SV(_base);
		SV(_bank);
		SV(_mode);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(!_locked) {
			_base = ((addr & 0x60) >> 2) | ((addr & 0x100) >> 3);
			_mode = (addr & 0x80) == 0x80;
			_locked = (addr & 0x2000) == 0x2000;
			
			SetMirroringType(addr & 0x02 ? MirroringType::Horizontal : MirroringType::Vertical);
		}

		_bank = value & 0x07;

		SelectPrgPage(0, _base | _bank);
		SelectPrgPage(1, _base | (_mode ? _bank : 7));
	}
};