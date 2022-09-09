#pragma once

#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_134 : public MMC3
{
private:
	uint8_t _exReg = 0;

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		BaseMapper::SelectChrPage(slot, (page & 0xFF) | ((_exReg & 0x20) << 3), memoryType);
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		BaseMapper::SelectPrgPage(slot, (page & 0x1F) | ((_exReg & 0x02) << 4), memoryType);
	}

	void InitMapper() override 
	{
		MMC3::InitMapper();

		AddRegisterRange(0x6001, 0x6001, MemoryOperation::Write);
	}

	void Reset(bool softReset) override 
	{
		BaseMapper::Reset(softReset);

		if(softReset) {
			_exReg = 0;
			MMC3::UpdateState();
		}
	}
	
	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_exReg);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr == 0x6001) {
			_exReg = value;
			MMC3::UpdateState();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};