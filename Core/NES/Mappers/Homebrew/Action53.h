#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "Utilities/Serializer.h"

class Action53 : public BaseMapper
{
private:
	uint8_t _selectedReg = 0;
	uint8_t _regs[4] = {};
	uint8_t _mirroringBit = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_selectedReg = 0;
		_mirroringBit = 0;
		memset(_regs, 0, sizeof(_regs));

		AddRegisterRange(0x5000, 0x5FFF, MemoryOperation::Write);

		SelectPrgPage(1, -1);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_regs, 4);
		SV(_selectedReg);
		SV(_mirroringBit);
	}

	void UpdateState()
	{
		uint8_t mirroring = _regs[2] & 0x03;
		if(!(mirroring & 0x02)) {
			mirroring = _mirroringBit;
		}

		switch(mirroring) {
			case 0: SetMirroringType(MirroringType::ScreenAOnly); break;
			case 1: SetMirroringType(MirroringType::ScreenBOnly); break;
			case 2: SetMirroringType(MirroringType::Vertical); break;
			case 3: SetMirroringType(MirroringType::Horizontal); break;
		}

		uint8_t gameSize = (_regs[2] & 0x30) >> 4;
		uint8_t prgSize = (_regs[2] & 0x08) >> 3;
		uint8_t slotSelect = (_regs[2] & 0x04) >> 2;
		uint8_t chrSelect = _regs[0] & 0x03;
		uint8_t prgSelect = _regs[1] & 0x0F;
		uint16_t outerPrgSelect = _regs[3] << 1;

		SelectChrPage(0, chrSelect);

		if(prgSize) {
			uint8_t bank = (slotSelect ? 0 : 1);
			switch(gameSize) {
				case 0: SelectPrgPage(bank, (outerPrgSelect & 0x1FE) | (prgSelect & 0x01)); break;
				case 1: SelectPrgPage(bank, (outerPrgSelect & 0x1FC) | (prgSelect & 0x03)); break;
				case 2: SelectPrgPage(bank, (outerPrgSelect & 0x1F8) | (prgSelect & 0x07)); break;
				case 3: SelectPrgPage(bank, (outerPrgSelect & 0x1F0) | (prgSelect & 0x0F)); break;
			}
			SelectPrgPage(slotSelect ? 1 : 0, (outerPrgSelect & 0x1FE) | slotSelect);
		} else {
			prgSelect <<= 1;
			uint16_t outerAnd[4]{ 0x1FE, 0x1FC, 0x1F8, 0x1F0 };
			uint8_t innerAnd[4]{ 0x01, 0x03, 0x07, 0x0F };
			SelectPrgPage(0, (outerPrgSelect & outerAnd[gameSize]) | (prgSelect & innerAnd[gameSize]));
			SelectPrgPage(1, (outerPrgSelect & outerAnd[gameSize]) | ((prgSelect | 0x01) & innerAnd[gameSize]));
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr <= 0x5FFF) {
			_selectedReg = ((value & 0x80) >> 6) | (value & 0x01);
		} else if(addr >= 0x8000) {
			if(_selectedReg <= 1) {
				_mirroringBit = (value >> 4) & 0x01;
			} else if(_selectedReg == 2) {
				_mirroringBit = (value & 0x01);
			}

			_regs[_selectedReg] = value;
			UpdateState();
		}
	}
};
