#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_123 : public MMC3
{
private:
	static constexpr uint8_t _security[8] = { 0,3,1,5,6,7,2,4 };
	uint8_t _exReg[2] = {};

	void UpdatePrgMapping() override 
	{
		if(_exReg[0] & 0x40) {
			uint8_t bank = (_exReg[0] & 0x05) | ((_exReg[0] & 0x08) >> 2) | ((_exReg[0] & 0x20) >> 2);
			if(_exReg[0] & 0x02) {
				SelectPrgPage4x(0, (bank & 0xFE) << 1);
			} else {
				SelectPrgPage2x(0, bank << 1);
				SelectPrgPage2x(1, bank << 1);
			}
		} else {
			MMC3::UpdatePrgMapping();
		}
	}

	void InitMapper() override
	{
		MMC3::InitMapper();

		_exReg[0] = _exReg[1] = 0;
		AddRegisterRange(0x5001, 0x5FFF, MemoryOperation::Write);
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_exReg[0]);
		SV(_exReg[1]);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000 && addr & 0x0800) {
			if(addr & 0x01) {
				_exReg[1] = value;
			} else {
				_exReg[0] = value;
			}
			UpdatePrgMapping();
		} else if(addr < 0xA000) {
			switch(addr & 0x8001) {
				case 0x8000: MMC3::WriteRegister(0x8000, (value & 0xC0) | (_security[value & 0x07])); break;
				case 0x8001: MMC3::WriteRegister(0x8001, value); break;
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};