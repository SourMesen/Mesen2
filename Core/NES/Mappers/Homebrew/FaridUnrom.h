#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class FaridUnrom : public BaseMapper
{
private:
	uint8_t _reg = 0;

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	bool HasBusConflicts() override { return true; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectPRGPage(1, 7);

		SelectCHRPage(0, 0);
	}

	void Reset(bool softReset) override
	{
		BaseMapper::Reset(softReset);

		if(softReset) {
			_reg = _reg & 0x87;
		} else {
			_reg = 0;
		}
	}
	
	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_reg);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		bool locked = _reg & 0x08;
		if(!locked && (_reg & 0x80) == 0 && (value & 0x80)) {
			//Latch bits
			_reg = (_reg & 0x87) | (value & 0x78);
		}
		_reg = (_reg & 0x78) | (value & 0x87);

		uint8_t outer = _reg & 0x70;
		
		SelectPRGPage(0, (_reg & 0x07) | (outer >> 1));
		SelectPRGPage(1, 0x07 | (outer >> 1));
	}
};