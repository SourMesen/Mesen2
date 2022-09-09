#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_245 : public MMC3
{
protected:
	void UpdateState() override
	{
		MMC3::UpdateState();

		if(HasChrRam()) {
			if(_chrMode) {
				SelectChrPage4x(0, 4);
				SelectChrPage4x(1, 0);
			} else {
				SelectChrPage4x(0, 0);
				SelectChrPage4x(1, 4);
			}
		}
	}

	void UpdatePrgMapping() override
	{
		uint8_t orValue = _registers[0] & 0x02 ? 0x40 : 0x00;
		_registers[6] = (_registers[6] & 0x3F) | orValue;
		_registers[7] = (_registers[7] & 0x3F) | orValue;

		uint16_t lastPageInBlock = (GetPrgPageCount() >= 0x40 ? (0x3F | orValue) : -1);
		if(_prgMode == 0) {
			SelectPrgPage(0, _registers[6]);
			SelectPrgPage(1, _registers[7]);
			SelectPrgPage(2, lastPageInBlock - 1);
			SelectPrgPage(3, lastPageInBlock);
		} else if(_prgMode == 1) {
			SelectPrgPage(0, lastPageInBlock - 1);
			SelectPrgPage(1, _registers[7]);
			SelectPrgPage(2, _registers[6]);
			SelectPrgPage(3, lastPageInBlock);
		}
	}
};