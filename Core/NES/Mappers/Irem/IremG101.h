#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class IremG101 : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x0400; }

	uint8_t _prgRegs[2] = {};
	uint8_t _prgMode = 0;

	void InitMapper() override
	{
		_prgRegs[0] = _prgRegs[1] = 0;
		_prgMode = 0;

		SelectPrgPage(2, -2);
		SelectPrgPage(3, -1);

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
			SelectPrgPage(0, _prgRegs[0]);
			SelectPrgPage(1, _prgRegs[1]);
			SelectPrgPage(2, -2);
			SelectPrgPage(3, -1);
		} else {
			SelectPrgPage(0, -2);
			SelectPrgPage(1, _prgRegs[1]);
			SelectPrgPage(2, _prgRegs[0]);
			SelectPrgPage(3, -1);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xF000) {
			case 0x8000:
				_prgRegs[0] = value & 0x1F;
				SelectPrgPage(_prgMode == 0 ? 0 : 2, _prgRegs[0]);
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
				SelectPrgPage(1, _prgRegs[1]);
				break;
			case 0xB000:
				SelectChrPage(addr & 0x07, value);
				break;
		}
	}
};