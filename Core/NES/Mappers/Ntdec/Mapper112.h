#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper112 : public BaseMapper
{
private:
	uint8_t _currentReg = 0;
	uint8_t _outerChrBank = 0;
	uint8_t _registers[8] = {};

protected:
	uint16_t RegisterStartAddress() override { return 0x8000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x400; }

	void InitMapper() override
	{
		_currentReg = 0;
		_outerChrBank = 0;
		memset(_registers, 0, sizeof(_registers));

		SetMirroringType(MirroringType::Vertical);
		AddRegisterRange(0x4020, 0x5FFF, MemoryOperation::Write);

		SelectPrgPage(2, -2);
		SelectPrgPage(3, -1);
		UpdateState();
	}
	
	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_registers, 8);
		SV(_currentReg);
		SV(_outerChrBank);
	}

	void UpdateState()
	{
		SelectPrgPage(0, _registers[0]);
		SelectPrgPage(1, _registers[1]);

		SelectChrPage2x(0, _registers[2]);
		SelectChrPage2x(1, _registers[3]);
		SelectChrPage(4, _registers[4] | ((_outerChrBank & 0x10) << 4));
		SelectChrPage(5, _registers[5] | ((_outerChrBank & 0x20) << 3));
		SelectChrPage(6, _registers[6] | ((_outerChrBank & 0x40) << 2));
		SelectChrPage(7, _registers[7] | ((_outerChrBank & 0x80) << 1));
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xE001) {
			case 0x8000: _currentReg = value & 0x07; break;
			case 0xA000: _registers[_currentReg] = value; break;
			case 0xC000: _outerChrBank = value; break;
			case 0xE000: SetMirroringType(value & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical); break;
		}
	
		UpdateState();
	}
};