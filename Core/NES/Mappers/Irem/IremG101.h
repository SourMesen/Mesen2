#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class IremG101 : public BaseMapper
{
protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x0400; }

	uint8_t _prgRegs[2] = {};
	uint8_t _prgMode = 0;

	void InitMapper() override
	{
		_prgRegs[0] = _prgRegs[1] = 0;
		_prgMode = 0;

		SelectPRGPage(2, -2);
		SelectPRGPage(3, -1);

		if(_romInfo.SubMapperID == 1) {
			//032: 1 Major League
			//CIRAM A10 is tied high (fixed one-screen mirroring) and PRG banking style is fixed as 8+8+16F 
			SetMirroringType(MirroringType::ScreenAOnly);
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);

		SV(_prgMode);
		SV(_prgRegs[0]);
		SV(_prgRegs[1]);
	}

	void UpdatePrgMode()
	{
		if(_prgMode == 0) {
			SelectPRGPage(0, _prgRegs[0]);
			SelectPRGPage(1, _prgRegs[1]);
			SelectPRGPage(2, -2);
			SelectPRGPage(3, -1);
		} else {
			SelectPRGPage(0, -2);
			SelectPRGPage(1, _prgRegs[1]);
			SelectPRGPage(2, _prgRegs[0]);
			SelectPRGPage(3, -1);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xF000) {
			case 0x8000:
				_prgRegs[0] = value & 0x1F;
				SelectPRGPage(_prgMode == 0 ? 0 : 2, _prgRegs[0]);
				break;
			case 0x9000:
				_prgMode = (value & 0x02) >> 1;
				if(_romInfo.SubMapperID == 1) {
					_prgMode = 0;
				}
				UpdatePrgMode();
				SetMirroringType((value & 0x01) == 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
				break;
			case 0xA000:
				_prgRegs[1] = value & 0x1F;
				SelectPRGPage(1, _prgRegs[1]);
				break;
			case 0xB000:
				SelectCHRPage(addr & 0x07, value);
				break;
		}
	}
};