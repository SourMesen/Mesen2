#pragma once
#include "stdafx.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class Bmc8in1 : public MMC3
{
private:
	uint8_t _reg = 0;

protected:
	void InitMapper() override
	{
		_reg = 0;
		MMC3::InitMapper();
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_reg);
	}

	void SelectCHRPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		MMC3::SelectCHRPage(slot, ((_reg & 0x0C) << 5) | (page & 0x7F), memoryType);
	}

	void SelectPRGPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		if(_reg & 0x10) {
			MMC3::SelectPRGPage(slot, ((_reg & 0x0C) << 2) | (page & 0x0F));
		} else {
			SelectPrgPage4x(0, (_reg & 0x0F) << 2);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr & 0x1000) {
			_reg = value;
			UpdateState();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};