#pragma once
#include "pch.h"
#include "NES/Mappers/Txc/Txc22211A.h"

class Txc22211C : public Txc22211A
{
protected:
	void UpdateState() override
	{
		SelectPrgPage(0, 0);
		if(_chrRomSize > 0x2000) {
			SelectChrPage(0, (_txc.GetOutput() & 0x01) | (_txc.GetY() ? 0x02 : 0) | ((_txc.GetOutput() & 0x02) << 1));
		} else {
			if(_txc.GetY()){
				SelectChrPage(0, 0);
			} else {
				RemovePpuMemoryMapping(0, 0x1FFF);
			}
		}
	}
};