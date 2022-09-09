#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Bmc830425C4391T : public BaseMapper
{
private:
	uint8_t _innerReg = 0;
	uint8_t _outerReg = 0;
	uint8_t _prgMode = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_innerReg = 0;
		_outerReg = 0;
		_prgMode = 0;

		SelectChrPage(0, 0);
		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_innerReg);
		SV(_outerReg);
		SV(_prgMode);
	}

	void UpdateState()
	{
		if(_prgMode) {
			//UNROM mode
			SelectPrgPage(0, (_innerReg & 0x07) | (_outerReg << 3));
			SelectPrgPage(1, 0x07 | (_outerReg << 3));
		} else {
			//UOROM mode
			SelectPrgPage(0, _innerReg | (_outerReg << 3));
			SelectPrgPage(1, 0x0F | (_outerReg << 3));
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_innerReg = value & 0x0F;
		if((addr & 0xFFE0) == 0xF0E0) {
			_outerReg = addr & 0x0F;
			_prgMode = (addr >> 4) & 0x01;
		}
		UpdateState();
	}
};
