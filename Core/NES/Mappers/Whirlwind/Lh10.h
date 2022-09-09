#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Lh10 : public BaseMapper
{
private:
	uint8_t _currentRegister = 0;
	uint8_t _regs[8] = {};

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	
	void InitMapper() override
	{
		memset(_regs, 0, sizeof(_regs));
		_currentRegister = 0;

		SelectChrPage(0, 0);
		RemoveRegisterRange(0xC000, 0xDFFF);

		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_regs, 8);
		SV(_currentRegister);
		
		if(!s.IsSaving()) {
			UpdateState();
		}
	}

	void UpdateState()
	{
		SetCpuMemoryMapping(0x6000, 0x7FFF, -2, PrgMemoryType::PrgRom);
		SelectPrgPage(0, _regs[6]);
		SelectPrgPage(1, _regs[7]);
		SelectPrgPage(2, 0, PrgMemoryType::WorkRam);
		SelectPrgPage(3, -1);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xE001) {
			case 0x8000: _currentRegister = value & 0x07; break;
			case 0x8001: _regs[_currentRegister] = value; UpdateState(); break;
		}
	}
};