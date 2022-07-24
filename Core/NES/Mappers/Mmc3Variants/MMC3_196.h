#pragma once
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_196 : public MMC3
{
private:
	uint8_t _exRegs[2] = {};

protected:
	void InitMapper() override
	{
		MMC3::InitMapper();
		_exRegs[0] = _exRegs[1] = 0;
		AddRegisterRange(0x6000, 0x6FFF, MemoryOperation::Write);
	}
	
	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_exRegs[0]);
		SV(_exRegs[1]);
	}

	void UpdatePrgMapping() override
	{
		if(_exRegs[0]) {
			//Used by Master Fighter II (Unl) (UT1374 PCB)
			SelectPrgPage4x(0, _exRegs[1] << 2);
		} else {
			MMC3::UpdatePrgMapping();
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override 
	{
		if(addr < 0x8000) {
			_exRegs[0] = 1;
			_exRegs[1] = (value & 0x0F) | (value >> 4);
			UpdatePrgMapping();
		} else {
			if(addr >= 0xC000) {
				addr = (addr & 0xFFFE) | ((addr >> 2) & 0x01) | ((addr >> 3) & 0x01);
			} else {
				addr = (addr & 0xFFFE) | ((addr >> 2) & 0x01) | ((addr >> 3) & 0x01) | ((addr >> 1) & 0x01);
			}
			MMC3::WriteRegister(addr, value);
		}
	}
};