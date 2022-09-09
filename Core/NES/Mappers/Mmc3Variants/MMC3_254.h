#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_254 : public MMC3
{
private:
	uint8_t _exRegs[2] = {};

protected:
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		MMC3::InitMapper();
		AddRegisterRange(0x6000, 0x7FFF, MemoryOperation::Read);
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_exRegs[0]);
		SV(_exRegs[1]);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		uint8_t value = InternalReadRam(addr);
		if(_exRegs[0]) {
			return value;
		} else {
			return value ^ _exRegs[1];
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr) {
			case 0x8000: _exRegs[0] = 0xFF; break;
			case 0xA001: _exRegs[1] = value; break;
		}
		MMC3::WriteRegister(addr, value);
	}
};