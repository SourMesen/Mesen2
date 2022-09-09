#pragma once

#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_187: public MMC3
{
private:
	uint8_t _exRegs[2] = {};

protected:
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		MMC3::InitMapper();

		memset(_exRegs, 0, sizeof(_exRegs));
		AddRegisterRange(0x5000, 0x5FFF, MemoryOperation::Any);
		AddRegisterRange(0x6000, 0x6FFF, MemoryOperation::Write);
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		if((_chrMode && slot >= 4) || (!_chrMode && slot < 4)) {
			page |= 0x100;
		}
		BaseMapper::SelectChrPage(slot, page);
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		if(!(_exRegs[0] & 0x80)) {
			BaseMapper::SelectPrgPage(slot, page & 0x3F, memoryType);
		} else {
			uint16_t exPage = _exRegs[0] & 0x1F;

			if(_exRegs[0] & 0x20) {
				if(_exRegs[0] & 0x40) {
					exPage &= 0xFC;
					BaseMapper::SelectPrgPage(0, exPage);
					BaseMapper::SelectPrgPage(1, exPage +1);
					BaseMapper::SelectPrgPage(2, exPage +2);
					BaseMapper::SelectPrgPage(3, exPage +3);
				} else {
					exPage &= 0xFE;
					BaseMapper::SelectPrgPage(0, (exPage << 1));
					BaseMapper::SelectPrgPage(1, (exPage << 1) + 1);
					BaseMapper::SelectPrgPage(2, (exPage << 1) + 2);
					BaseMapper::SelectPrgPage(3, (exPage << 1) + 3);
				}
			} else {
				BaseMapper::SelectPrgPage(0, (exPage << 1));
				BaseMapper::SelectPrgPage(1, (exPage << 1) + 1);
				BaseMapper::SelectPrgPage(2, (exPage << 1));
				BaseMapper::SelectPrgPage(3, (exPage << 1) + 1);
			}
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		uint8_t security[4] = { 0x83,0x83,0x42,0x00 };
		return security[_exRegs[1] & 0x03];
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if(addr == 0x5000 || addr == 0x6000) {
				_exRegs[0] = value;
				MMC3::UpdatePrgMapping();
			}
		} else {
			if(addr == 0x8000) {
				_exRegs[1] = 1;
				MMC3::WriteRegister(addr, value);
			} else if(addr == 0x8001) {
				if(_exRegs[1] == 1) {
					MMC3::WriteRegister(addr, value);
				}
			} else {
				MMC3::WriteRegister(addr, value);
			}
		}
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_exRegs[0]);
		SV(_exRegs[1]);
	}
};