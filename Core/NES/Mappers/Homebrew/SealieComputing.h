#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class SealieComputing : public BaseMapper
{
protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint32_t GetWorkRamSize() override { return 0x2000; }
	uint32_t GetChrRamSize() override { return 0x8000; }
	uint16_t RegisterStartAddress() override { return 0x8000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		SelectPrgPage(1, -1);

		//"It is hard-wired for vertical mirroring", but no need to enforce this, just need proper iNES headers.
		//SetMirroringType(MirroringType::Vertical);

		//"contains 8KB of WRAM mounted in the usual place"
		SetCpuMemoryMapping(0x6000, 0x7FFF, 0, PrgMemoryType::WorkRam, MemoryAccessType::ReadWrite);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectChrPage(0, value & 0x03);
		SelectPrgPage(0, (value >> 2) & 0x07);
	}
};