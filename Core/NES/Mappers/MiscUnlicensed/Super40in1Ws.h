#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Super40in1Ws : public BaseMapper
{
private:
	bool _regLock = false;

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0x6FFF; }

	void InitMapper() override
	{
		_regLock = false;
		WriteRegister(0x6000, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_regLock);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(!_regLock) {
			if(addr & 0x01) {
				SelectCHRPage(0, value);
			} else {
				_regLock = (value & 0x20) == 0x20;

				SelectPRGPage(0, value & ~(~value >> 3 & 0x01));
				SelectPRGPage(1, value | (~value >> 3 & 0x01));
				SetMirroringType((value & 0x10) ? MirroringType::Horizontal : MirroringType::Vertical);
			}
		}
	}
};