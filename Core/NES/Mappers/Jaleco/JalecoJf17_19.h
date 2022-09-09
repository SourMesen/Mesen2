#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

//Note: Audio bits not supported
class JalecoJf17_19 : public BaseMapper
{
private:
	bool _jf19Mode = false;
	bool _prgFlag = false;
	bool _chrFlag = false;

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	bool HasBusConflicts() override { return true; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectPrgPage(1, -1);

		SelectChrPage(0, 0);
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
				SelectPrgPage(1, value & 0x0F);
			} else {
				SelectPrgPage(0, value & 0x07);
			}
		}

		if(!_chrFlag && (value & 0x40)) {
			SelectChrPage(0, value & 0x0F);
		}
		
		_prgFlag = (value & 0x80) == 0x80;
		_chrFlag = (value & 0x40) == 0x40;
	}

public:
	JalecoJf17_19(bool jf19Mode) : _jf19Mode(jf19Mode)
	{
	}
};