#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_249 : public MMC3
{
private:
	uint8_t _exReg = 0;

protected:
	void InitMapper() override
	{
		MMC3::InitMapper();

		AddRegisterRange(0x5000, 0x5000, MemoryOperation::Write);
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_exReg);
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		if(_exReg & 0x02) {
			page = (page & 0x03) | ((page >> 1) & 0x04) | ((page >> 4) & 0x08) | ((page >> 2) & 0x10) | ((page << 3) & 0x20) | ((page << 2) & 0xC0);
		}

		BaseMapper::SelectChrPage(slot, page, memoryType);
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		if(_exReg & 0x02) {
			if(page < 0x20) {
				page = (page & 0x01) | ((page >> 3) & 0x02) | ((page >> 1) & 0x04) | ((page << 2) & 0x18);
			} else {
				page -= 0x20;
				page = (page & 0x03) | ((page >> 1) & 0x04) | ((page >> 4) & 0x08) | ((page >> 2) & 0x10) | ((page << 3) & 0x20) | ((page << 2) & 0xC0);
			}
		}

		BaseMapper::SelectPrgPage(slot, page, memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr == 0x5000) {
			_exReg = value;
			UpdatePrgMapping();
			UpdateChrMapping();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};