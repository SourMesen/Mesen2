#pragma once

#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_115 : public MMC3
{
private:
	uint8_t _prgReg = 0;
	uint8_t _chrReg = 0;
	uint8_t _protectionReg = 0;

protected:
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		AddRegisterRange(0x4100, 0x7FFF, MemoryOperation::Write);
		AddRegisterRange(0x5000, 0x5FFF, MemoryOperation::Read);
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);

		MMC3::InitMapper();
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		page |= (_chrReg << 8);
		BaseMapper::SelectChrPage(slot, page);
	}

	void UpdateState() override
	{
		MMC3::UpdateState();

		if(_prgReg & 0x80) {
			if(_prgReg & 0x20) {
				SelectPrgPage4x(0, ((_prgReg & 0x0F) >> 1) << 2);				
			} else {
				SelectPrgPage2x(0, (_prgReg & 0x0F) << 1);
				SelectPrgPage2x(1, (_prgReg & 0x0F) << 1);
			}
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		return _protectionReg;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if(addr == 0x5080) {
				_protectionReg = value;
			} else {
				if(addr & 0x01) {
					_chrReg = value & 0x01;
				} else {
					_prgReg = value;
				}
				UpdateState();
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
	
	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_prgReg);
		SV(_chrReg);
	}
};