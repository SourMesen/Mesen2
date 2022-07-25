#pragma once
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_219 : public MMC3
{
private:
	uint8_t _exRegs[3] = {};

protected:
	void InitMapper() override
	{
		MMC3::InitMapper();
		SelectPrgPage4x(0, -4);
		SelectChrPage8x(0, 0);
		_exRegs[0] = 0;
		_exRegs[1] = 0;
		_exRegs[2] = 0;
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SVArray(_exRegs, 3);
	}

	void UpdatePrgMapping() override
	{
	}

	void UpdateChrMapping() override
	{
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0xA000) {
			switch(addr & 0xE003) {
				case 0x8000: 
					_exRegs[0] = 0;
					_exRegs[1] = value;
					break;

				case 0x8001:
					if(_exRegs[0] >= 0x23 && _exRegs[0] <= 0x26) {
						uint8_t prgBank = ((value & 0x20) >> 5) | ((value & 0x10) >> 3) | ((value & 0x08) >> 1) | ((value & 0x04) << 1);
						SelectPrgPage(0x26 - _exRegs[0], prgBank);
					}

					switch(_exRegs[1]) {
						case 0x08: case 0x0A: case 0x0E: case 0x12: case 0x16: case 0x1A:	case 0x1E: 
							_exRegs[2] = value << 4; 
							break;

						case 0x09: SelectChrPage(0, _exRegs[2] | (value >> 1 & 0x0E)); break;
						case 0x0B: SelectChrPage(1, _exRegs[2] | (value >> 1 | 0x1)); break;
						case 0x0C:
						case 0x0D: SelectChrPage(2, _exRegs[2] | (value >> 1 & 0xE)); break;
						case 0x0F: SelectChrPage(3, _exRegs[2] | (value >> 1 | 0x1)); break;
						case 0x10:
						case 0x11: SelectChrPage(4, _exRegs[2] | (value >> 1 & 0xF)); break;
						case 0x14:
						case 0x15: SelectChrPage(5, _exRegs[2] | (value >> 1 & 0xF)); break;
						case 0x18:
						case 0x19: SelectChrPage(6, _exRegs[2] | (value >> 1 & 0xF)); break;
						case 0x1C:
						case 0x1D: SelectChrPage(7, _exRegs[2] | (value >> 1 & 0xF)); break;
					}
					break;

				case 0x8002:
					_exRegs[0] = value;
					_exRegs[1] = 0;
					break;
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};