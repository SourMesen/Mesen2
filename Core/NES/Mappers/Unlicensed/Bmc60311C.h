#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Bmc60311C : public BaseMapper
{
private:
	uint8_t _innerPrg = 0;
	uint8_t _outerPrg = 0;
	uint8_t _mode = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		_innerPrg = 0;
		_outerPrg = 0;
		_mode = 0;

		UpdateState();
		SelectChrPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_innerPrg);
		SV(_outerPrg);
		SV(_mode);
	}

	void UpdateState()
	{
		uint8_t page = _outerPrg | ((_mode & 0x04) ? 0 : _innerPrg);

		switch(_mode & 0x03) {
			case 0:
				//0: NROM-128: Same inner/outer 16 KiB bank at CPU $8000-$BFFF and $C000-$FFFF
				SelectPrgPage(0, page);
				SelectPrgPage(1, page);
				break;

			case 1:
				//1: NROM-256: 32 kiB bank at CPU $8000-$FFFF (Selected inner/outer bank SHR 1)
				SelectPrgPage2x(0, page & 0xFE);
				break;

			case 2:
				//2: UNROM: Inner/outer bank at CPU $8000-BFFF, fixed inner bank 7 within outer bank at $C000-$FFFF
				SelectPrgPage(0, page);
				SelectPrgPage(1, _outerPrg | 7);
				break;

			case 3:
				//Unknown
				break;
		}

		SetMirroringType(_mode & 0x08 ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr >= 0x8000) {
			_innerPrg = value & 0x07;
			UpdateState();
		} else {
			switch(addr & 0xE001) {
				case 0x6000:
					_mode = value & 0x0F;
					UpdateState();
					break;

				case 0x6001:
					_outerPrg = value;
					UpdateState();
					break;
			}
		}
	}
};