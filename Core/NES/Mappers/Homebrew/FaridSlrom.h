#pragma once
#include "NES/Mappers/Nintendo/MMC1.h"

class FaridSlrom : public MMC1
{
private:
	uint8_t _outerBank = 0;
	bool _locked = false;

protected:
	void InitMapper() override
	{
		AddRegisterRange(0x6000, 0x7FFF, MemoryOperation::Write);
		MMC1::InitMapper();
	}

	void Reset(bool softReset) override
	{
		MMC1::Reset(softReset);

		_outerBank = 0;
		_locked = false;
		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		MMC1::Serialize(s);
		SV(_outerBank);
		SV(_locked);
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType) override
	{
		MMC1::SelectChrPage(slot, (_outerBank << 2) | (page & 0x1F), memoryType);
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType) override
	{
		MMC1::SelectPrgPage(slot, _outerBank | (page & 0x07), memoryType);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			bool wramEnabled = !_wramDisable;
			if(wramEnabled && !_locked) {
				_outerBank = (value & 0x70) >> 1;
				_locked = (value & 0x08) == 0x08;
				UpdateState();
			}
		} else {
			MMC1::WriteRegister(addr, value);
		}
	}
};