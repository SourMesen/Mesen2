#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_StreetHeroes : public MMC3
{
private:
	uint8_t _exReg = 0;
	uint8_t _resetSwitch = 0;

protected:
	uint16_t GetChrRamPageSize() override { return 0x2000; }
	uint32_t GetChrRamSize() override { return 0x2000; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		_exReg = 0;
		_resetSwitch = 0;

		MMC3::InitMapper();

		AddRegisterRange(0x4100, 0x4100, MemoryOperation::Any);
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_exReg);
		SV(_resetSwitch);
	}

	void Reset(bool softReset) override
	{
		if(softReset) {
			_resetSwitch ^= 0xFF;
		}
		UpdateState();
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		if(_exReg & 0x40) {
			MMC3::SelectChrPage(0, 0, ChrMemoryType::ChrRam);
		} else {
			switch(slot) {
				case 0: case 1: MMC3::SelectChrPage(slot, page | ((_exReg & 0x08) << 5)); break;
				case 2: case 3: MMC3::SelectChrPage(slot, page | ((_exReg & 0x04) << 6)); break;
				case 4: case 5: MMC3::SelectChrPage(slot, page | ((_exReg & 0x01) << 8)); break;
				default: MMC3::SelectChrPage(slot, page | ((_exReg & 0x02) << 7)); break;
			}
		}
	}
		
	uint8_t ReadRegister(uint16_t addr) override
	{
		return _resetSwitch;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr == 0x4100) {
			_exReg = value;
			UpdateState();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};