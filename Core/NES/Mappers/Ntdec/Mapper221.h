#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper221 : public BaseMapper
{
private:
	uint16_t _mode = 0;
	uint8_t _prgReg = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_prgReg = 0;
		_mode = 0;

		SelectChrPage(0, 0);

		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_mode);
		SV(_prgReg);
	}

	void UpdateState()
	{
		uint16_t outerBank = (_mode & 0xFC) >> 2;
		if(_mode & 0x02) {
			if(_mode & 0x0100) {
				SelectPrgPage(0, outerBank | _prgReg);
				SelectPrgPage(1, outerBank | 0x07);
			} else {
				SelectPrgPage2x(0, outerBank | (_prgReg & 0x06));
			}
		} else {
			SelectPrgPage(0, outerBank | _prgReg);
			SelectPrgPage(1, outerBank | _prgReg);
		}

		SetMirroringType(_mode & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xC000) {
			case 0x8000:
				_mode = addr;
				UpdateState();
				break;

			case 0xC000:
				_prgReg = addr & 0x07;
				UpdateState();
				break;
		}
	}
};