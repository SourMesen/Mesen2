#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_14 : public MMC3
{
private:
	uint8_t _vrcChrRegs[8] = {};
	uint8_t _vrcPrgRegs[2] = {};
	uint8_t _vrcMirroring = 0;
	uint8_t _mode = 0;

protected:
	void InitMapper() override
	{
		_mode = 0;
		_vrcMirroring = 0;
		memset(_vrcPrgRegs, 0, sizeof(_vrcPrgRegs));
		memset(_vrcChrRegs, 0, sizeof(_vrcChrRegs));

		MMC3::InitMapper();
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SVArray(_vrcPrgRegs, 2);
		SVArray(_vrcChrRegs, 8);
		SV(_mode);
		SV(_vrcMirroring);
	}

	void UpdateChrMapping() override
	{
		int slotSwap = (GetState().Reg8000 & 0x80) ? 4 : 0;
		int outerBank0 = (_mode & 0x08) ? 0x100 : 0;
		int outerBank1 = (_mode & 0x20) ? 0x100 : 0;
		int outerBank2 = (_mode & 0x80) ? 0x100 : 0;
		SelectChrPage(0 ^ slotSwap, outerBank0 | (_registers[0] & (~1)));
		SelectChrPage(1 ^ slotSwap, outerBank0 | _registers[0] | 1);
		SelectChrPage(2 ^ slotSwap, outerBank0 | (_registers[1] & (~1)));
		SelectChrPage(3 ^ slotSwap, outerBank0 | _registers[1] | 1);
		SelectChrPage(4 ^ slotSwap, outerBank1 | _registers[2]);
		SelectChrPage(5 ^ slotSwap, outerBank1 | _registers[3]);
		SelectChrPage(6 ^ slotSwap, outerBank2 | _registers[4]);
		SelectChrPage(7 ^ slotSwap, outerBank2 | _registers[5]);
	}

	void UpdateVrcState()
	{
		SelectPrgPage(0, _vrcPrgRegs[0]);
		SelectPrgPage(1, _vrcPrgRegs[1]);
		SelectPrgPage(2, -2);
		SelectPrgPage(3, -1);

		for(int i = 0; i < 8; i++) {
			SelectChrPage(i, _vrcChrRegs[i]);
		}
		
		SetMirroringType(_vrcMirroring & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr == 0xA131) {
			_mode = value;
		}

		if(_mode & 0x02) {
			MMC3::UpdateState();
			MMC3::WriteRegister(addr, value);
		} else {
			if(addr >= 0xB000 && addr <= 0xEFFF) {
				uint8_t regNumber = ((((addr >> 12) & 0x07) - 3) << 1) + ((addr >> 1) & 0x01);
				bool lowBits = (addr & 0x01) == 0x00;
				if(lowBits) {
					_vrcChrRegs[regNumber] = (_vrcChrRegs[regNumber] & 0xF0) | (value & 0x0F);
				} else {
					_vrcChrRegs[regNumber] = (_vrcChrRegs[regNumber] & 0x0F) | ((value & 0x0F) << 4);
				}
			} else {
				switch(addr & 0xF003) {
					case 0x8000: _vrcPrgRegs[0] = value; break;
					case 0x9000: _vrcMirroring = value; break;
					case 0xA000: _vrcPrgRegs[1] = value; break;
				}
			}
			UpdateVrcState();
		}
	}
};