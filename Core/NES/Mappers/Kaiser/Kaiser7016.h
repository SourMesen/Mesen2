#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"

class Kaiser7016 : public BaseMapper
{
	uint8_t _prgReg = 0;

protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_prgReg = 8;

		SelectPRGPage(0, 0x0C);
		SelectPRGPage(1, 0x0D);
		SelectPRGPage(2, 0x0E);
		SelectPRGPage(3, 0x0F);
		SelectCHRPage(0, 0 );

		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_prgReg);

		if(!s.IsSaving()) {
			UpdateState();
		}
	}

	void UpdateState()
	{
		SetCpuMemoryMapping(0x6000, 0x7FFF, _prgReg, PrgMemoryType::PrgRom);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		bool mode = (addr & 0x30) == 0x30;
		switch(addr & 0xD943) {
			case 0xD943: {
				if(mode) {
					_prgReg = 0x0B;
				} else {
					_prgReg = (addr >> 2) & 0x0F;
				}
				UpdateState();
				break;
			}
			case 0xD903: {
				if(mode) {
					_prgReg = 0x08 | ((addr >> 2) & 0x03);
				} else {
					_prgReg = 0x0B;
				}
				UpdateState();
				break;
			}
		}
	}
};