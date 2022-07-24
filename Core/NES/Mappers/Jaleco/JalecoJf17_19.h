#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

//Note: Audio bits not supported
class JalecoJf17_19 : public BaseMapper
{
private:
	bool _jf19Mode = false;
	bool _prgFlag = false;
	bool _chrFlag = false;

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	bool HasBusConflicts() override { return true; }

	void InitMapper() override
	{
		SelectPRGPage(0, 0);
		SelectPRGPage(1, -1);

		SelectCHRPage(0, 0);
	}

	virtual void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_prgFlag);
		SV(_chrFlag);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(!_prgFlag && (value & 0x80)) {
			if(_jf19Mode) {
				SelectPRGPage(1, value & 0x0F);
			} else {
				SelectPRGPage(0, value & 0x07);
			}
		}

		if(!_chrFlag && (value & 0x40)) {
			SelectCHRPage(0, value & 0x0F);
		}
		
		_prgFlag = (value & 0x80) == 0x80;
		_chrFlag = (value & 0x40) == 0x40;
	}

public:
	JalecoJf17_19(bool jf19Mode) : _jf19Mode(jf19Mode)
	{
	}
};