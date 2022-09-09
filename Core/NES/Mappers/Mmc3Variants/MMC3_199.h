#pragma once

#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_199 : public MMC3
{
private:
	uint8_t _exRegs[4] = {};

protected:
	uint32_t GetChrRamSize() override { return 0x2000; }
	uint16_t GetChrRamPageSize() override { return 0x400; }

	void InitMapper() override
	{
		_exRegs[0] = 0xFE;
		_exRegs[1] = 0xFF;
		_exRegs[2] = 1;
		_exRegs[3] = 3;

		MMC3::InitMapper();
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SVArray(_exRegs, 4);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr == 0x8001 && (GetState().Reg8000 & 0x08)) {
			_exRegs[GetState().Reg8000 & 0x03] = value;
			UpdatePrgMapping();
			UpdateChrMapping();
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}

	void UpdateMirroring() override
	{
		switch(GetState().RegA000 & 0x03) {
			case 0: SetMirroringType(MirroringType::Vertical); break;
			case 1: SetMirroringType(MirroringType::Horizontal); break;
			case 2: SetMirroringType(MirroringType::ScreenAOnly); break;
			case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
		}
	}

	void UpdatePrgMapping() override
	{
		MMC3::UpdatePrgMapping();
		SelectPrgPage(2, _exRegs[0]);
		SelectPrgPage(3, _exRegs[1]);
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType) override
	{
		MMC3::SelectChrPage(slot, page, page < 8 ? ChrMemoryType::ChrRam : ChrMemoryType::ChrRom);

		MMC3::SelectChrPage(0, _registers[0], _registers[0] < 8 ? ChrMemoryType::ChrRam : ChrMemoryType::ChrRom);
		MMC3::SelectChrPage(1, _exRegs[2], _exRegs[2] < 8 ? ChrMemoryType::ChrRam : ChrMemoryType::ChrRom);
		MMC3::SelectChrPage(2, _registers[1], _registers[1] < 8 ? ChrMemoryType::ChrRam : ChrMemoryType::ChrRom);
		MMC3::SelectChrPage(3, _exRegs[3], _exRegs[3] < 8 ? ChrMemoryType::ChrRam : ChrMemoryType::ChrRom);
	}
};