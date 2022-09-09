#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Bb : public BaseMapper
{
private:
	uint8_t _prgReg = 0;
	uint8_t _chrReg = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_prgReg = -1;
		_chrReg = 0;

		SelectPrgPage4x(0, -4);
		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_prgReg);
		SV(_chrReg);
		if(!s.IsSaving()) {
			UpdateState();
		}
	}

	void UpdateState()
	{
		SetCpuMemoryMapping(0x6000, 0x7FFF, _prgReg, PrgMemoryType::PrgRom);
		SelectChrPage(0, _chrReg);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if((addr & 0x9000) == 0x8000 || addr >= 0xF000){
			//A version of Bubble Bobble expects writes to $F000+ to switch the PRG banks
			_prgReg = _chrReg = value;
		} else {
			//For ProWres
			_chrReg = value & 0x01;
		}
		UpdateState();
	}
};