#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

//Most likely incorrect/incomplete, but works (with minor glitches) with the 2 games marked as mapper 198 that I am aware of.
//Game 1: 吞食天地2  (CHR RAM, but uses chr banking?, has save ram at 6000-7FFF?)
//Game 2: Cheng Ji Si Han (ES-1110) (Ch)  (CHR RAM, work ram mirrored from 5000-7FFF?, doesn't use chr banking)
//These games may actually use different mappers.
class MMC3_198 : public MMC3
{
private:
	uint8_t _exRegs[4] = {};

protected:
	uint32_t GetWorkRamSize() override { return 0x1000; }
	uint32_t GetWorkRamPageSize() override { return 0x1000; }
	uint16_t GetChrRamPageSize() override { return 0x400; }
	bool ForceWorkRamSize() override { return true; }

	void InitMapper() override
	{
		_exRegs[0] = 0;
		_exRegs[1] = 1;
		_exRegs[2] = GetPrgPageCount() - 2;
		_exRegs[3] = GetPrgPageCount() - 1;

		//Set 4kb of work ram at $5000, mirrored
		SetCpuMemoryMapping(0x5000, 0x7FFF, 0, PrgMemoryType::WorkRam);

		MMC3::InitMapper();

		if(GetSaveRamSize() == 0) {
			SetCpuMemoryMapping(0x5000, 0x7FFF, 0, PrgMemoryType::WorkRam);
		}
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SVArray(_exRegs, 4);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr == 0x8001 && (GetState().Reg8000 & 0x07) >= 6) {
			_exRegs[(GetState().Reg8000 & 0x07) - 6] = value & (value >= 0x40 ? 0x4F : 0x3F);
		}
		MMC3::WriteRegister(addr, value);
	}
	
	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType) override
	{
		MMC3::SelectPrgPage(slot, _exRegs[slot], memoryType);
	}

	void UpdateChrMapping() override
	{
		if(_chrRamSize > 0 && _registers[0] == 0 && _registers[1] == 0 && _registers[2] == 0 && _registers[3] == 0 && _registers[4] == 0 && _registers[5] == 0) {
			SelectChrPage8x(0, 0, ChrMemoryType::ChrRam);
		} else {
			MMC3::UpdateChrMapping();
		}
	}
};