#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_197 : public MMC3
{
protected:
	void UpdateChrMapping() override
	{
		if(_chrMode == 0) {
			SelectChrPage4x(0, _registers[0] << 1);

			SelectChrPage2x(2, _registers[2] << 1);
			SelectChrPage2x(3, _registers[3] << 1);
		} else if(_chrMode == 1) {
			SelectChrPage4x(0, _registers[2] << 1);

			SelectChrPage2x(2, _registers[0] << 1);
			SelectChrPage2x(3, _registers[0] << 1);
		}
	}
};