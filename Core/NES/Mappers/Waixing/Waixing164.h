#pragma once
#include "NES/BaseMapper.h"

class Waixing164 : public BaseMapper
{
private:
	uint8_t _prgBank = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x5000; }
	uint16_t RegisterEndAddress() override { return 0x5FFF; }

	void InitMapper() override
	{
		_prgBank = 0x0F;
		SelectPrgPage(0, _prgBank);
		SelectChrPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_prgBank);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0x7300) {
			case 0x5000: _prgBank = (_prgBank & 0xF0) | (value & 0x0F); SelectPrgPage(0, _prgBank); break;
			case 0x5100: _prgBank = (_prgBank & 0x0F) | ((value & 0x0F) << 4); SelectPrgPage(0, _prgBank); break;
		}
	}
};