#pragma once
#include "stdafx.h"
#include "NES/Mappers/MMC3.h"

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

		uint16_t lastPageInBlock = (GetPRGPageCount() >= 0x40 ? (0x3F | orValue) : -1);
		if(_prgMode == 0) {
			SelectPRGPage(0, _registers[6]);
			SelectPRGPage(1, _registers[7]);
			SelectPRGPage(2, lastPageInBlock - 1);
			SelectPRGPage(3, lastPageInBlock);
		} else if(_prgMode == 1) {
			SelectPRGPage(0, lastPageInBlock - 1);
			SelectPRGPage(1, _registers[7]);
			SelectPRGPage(2, _registers[6]);
			SelectPRGPage(3, lastPageInBlock);
		}
	}
};