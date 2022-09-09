#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper226 : public BaseMapper
{
protected:
	uint8_t _registers[2] = {};

	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_registers[0] = 0;
		_registers[1] = 0;

		SelectPrgPage(0, 0);
		SelectPrgPage(1, 1);
		SelectChrPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_registers[0]);
		SV(_registers[1]);
	}

	void Reset(bool softReset) override
	{
		if(softReset) {
			_registers[0] = 0;
			_registers[1] = 0;

			SelectPrgPage(0, 0);
			SelectPrgPage(1, 1);
			SelectChrPage(0, 0);
		}
	}

	virtual uint8_t GetPrgPage()
	{
		return (_registers[0] & 0x1F) | ((_registers[0] & 0x80) >> 2) | ((_registers[1] & 0x01) << 6);
	}

	void UpdatePrg()
	{
		uint8_t prgPage = GetPrgPage();
		if(_registers[0] & 0x20) {
			SelectPrgPage(0, prgPage);
			SelectPrgPage(1, prgPage);
		} else {
			SelectPrgPage(0, prgPage & 0xFE);
			SelectPrgPage(1, (prgPage & 0xFE) + 1);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0x8001) {
			case 0x8000: _registers[0] = value;	break;
			case 0x8001: _registers[1] = value; break;
		}

		UpdatePrg();

		SetMirroringType(_registers[0] & 0x40 ? MirroringType::Vertical : MirroringType::Horizontal);
	}
};
