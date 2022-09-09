#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class ResetTxrom : public MMC3
{
private:
	uint8_t _resetCounter = 0;

protected:
	void Reset(bool softReset) override
	{
		MMC3::Reset(softReset);
		if(softReset) {
			_resetCounter = (_resetCounter + 1) & 0x03;
			UpdateState();
		} else {
			_resetCounter = 0;
		}
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_resetCounter);
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType) override
	{
		page = (_resetCounter << 7) | (page & 0x7F);
		MMC3::SelectChrPage(slot, page, memoryType);
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType) override
	{
		page = (_resetCounter << 4) | (page & 0x0F);
		MMC3::SelectPrgPage(slot, page, memoryType);
	}
};