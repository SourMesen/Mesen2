#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC2.h"

class MMC4 : public MMC2
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x1000; }
	bool EnableVramAddressHook() override { return true; }

	void InitMapper() override
	{
		_leftLatch = 1;
		_rightLatch = 1;
		_leftChrPage[0] = GetPowerOnByte() & 0x1F;
		_leftChrPage[1] = GetPowerOnByte() & 0x1F;
		_rightChrPage[0] = GetPowerOnByte() & 0x1F;
		_rightChrPage[1] = GetPowerOnByte() & 0x1F;
		_needChrUpdate = false;

		SelectPrgPage(1, -1);
	}

public:
	void NotifyVramAddressChange(uint16_t addr) override
	{
		if(_needChrUpdate) {
			SelectChrPage(0, _leftChrPage[_leftLatch]);
			SelectChrPage(1, _rightChrPage[_rightLatch]);
			_needChrUpdate = false;
		}

		if(addr >= 0x0FD8 && addr <= 0x0FDF) {
			_leftLatch = 0;
			_needChrUpdate = true;
		} else if(addr >= 0x0FE8 && addr <= 0x0FEF) {
			_leftLatch = 1;
			_needChrUpdate = true;
		} else if(addr >= 0x1FD8 && addr <= 0x1FDF) {
			_rightLatch = 0;
			_needChrUpdate = true;
		} else if(addr >= 0x1FE8 && addr <= 0x1FEF) {
			_rightLatch = 1;
			_needChrUpdate = true;
		}
	}
};