#pragma once
#include "NES/Mappers/Nintendo/MMC1.h"

class MMC1_155 : public MMC1
{
protected :
	void UpdateState()
	{
		//WRAM disable bit does not exist in mapper 155
		_wramDisable = false;

		MMC1::UpdateState();
	}
};