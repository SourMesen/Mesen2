#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_BmcF15 : public MMC3
{
private:
	uint8_t _exReg = 0;

protected:
	void InitMapper() override
	{
		AddRegisterRange(0x6000, 0xFFFF, MemoryOperation::Write);
		_exReg = 0;
		MMC3::InitMapper();
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_exReg);
	}

	void UpdatePrgMapping() override
	{
		uint32_t bank = _exReg & 0x0F;
		uint32_t mode = (_exReg & 0x08) >> 3;
		uint32_t mask = ~mode;
		SelectPrgPage2x(0, (bank & mask) << 1);
		SelectPrgPage2x(1, ((bank & mask) | mode) << 1);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if(GetState().RegA001 & 0x80) {
				_exReg = value & 0x0F;
				UpdatePrgMapping();
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};