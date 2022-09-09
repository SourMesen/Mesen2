#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Bmc51 : public BaseMapper
{
private:
	uint8_t _bank = 0;
	uint8_t _mode = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x6000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }

	void InitMapper() override
	{
		_bank = 0;
		_mode = 1;
		UpdateState();
	}
	
	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_bank);
		SV(_mode);
		if(!s.IsSaving()) {
			UpdateState();
		}
	}

	void UpdateState()
	{
		if(_mode & 0x01) {
			SelectPrgPage4x(0, _bank << 2);
			SetCpuMemoryMapping(0x6000, 0x7FFF, (0x23 | (_bank << 2)), PrgMemoryType::PrgRom);
		} else {
			SelectPrgPage2x(0, (_bank << 2) | _mode);
			SelectPrgPage2x(1, _bank << 2 | 0x0E);
			SetCpuMemoryMapping(0x6000, 0x7FFF, (0x2F | (_bank << 2)), PrgMemoryType::PrgRom);
		}

		SetMirroringType(_mode == 0x03 ? MirroringType::Horizontal : MirroringType::Vertical);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr <= 0x7FFF) {
			_mode = ((value >> 3) & 0x02) | ((value >> 1) & 0x01);
		} else if(addr >= 0xC000 && addr <= 0xDFFF) {
			_bank = value & 0x0F;
			_mode = ((value >> 3) & 0x02) | (_mode & 0x01);
		} else {
			_bank = value & 0x0F;
		}
		UpdateState();
	}
};