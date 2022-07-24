#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class GoldenFive : public BaseMapper
{
private:
	uint8_t _prgReg = 0;

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_prgReg = 0;
		SelectPRGPage(1, 0x0F);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_prgReg);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr >= 0xC000) {
			_prgReg = (_prgReg & 0xF0) | (value & 0x0F);
			SelectPRGPage(0, _prgReg);
		} else if(addr <= 0x9FFF) {
			if(value & 0x08) {
				_prgReg = (_prgReg & 0x0F) | ((value << 4) & 0x70);
				SelectPRGPage(0, _prgReg);
				SelectPRGPage(1, ((value << 4) & 0x70) | 0x0F);
			}
		}
	}
};