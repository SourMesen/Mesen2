#pragma once
#include "stdafx.h"
#include "NES/Mappers/MMC3.h"

class Bmc830118C : public MMC3
{
private:
	uint8_t _reg = 0;

protected:
	void InitMapper() override
	{
		_reg = 0;
		MMC3::InitMapper();
		AddRegisterRange(0x6800, 0x68FF, MemoryOperation::Write);
	}

	void Reset(bool softReset) override
	{
		_reg = 0;
		MMC3::Reset(softReset);
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
		if((_reg & 0x0C) == 0x0C) {
			if(slot == 0) {
				MMC3::SelectPRGPage(0, ((_reg & 0x0C) << 2) | (page & 0x0F));
				MMC3::SelectPRGPage(2, 0x32 | (page & 0x0F));
			} else if(slot == 1) {
				MMC3::SelectPRGPage(1, ((_reg & 0x0C) << 2) | (page & 0x0F));
				MMC3::SelectPRGPage(3, 0x32 | (page & 0x0F));
			}
		} else {
			MMC3::SelectPRGPage(slot, ((_reg & 0x0C) << 2) | (page & 0x0F));
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_reg = value;
			UpdateState();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};