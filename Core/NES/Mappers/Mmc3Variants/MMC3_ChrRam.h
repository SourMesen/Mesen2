#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class MMC3_ChrRam : public MMC3
{
private:
	uint16_t _firstRamBank = 0;
	uint16_t _lastRamBank = 0;
	uint16_t _chrRamSize = 0;

protected:
	uint16_t GetChrRamPageSize() override { return 0x400; }
	uint32_t GetChrRamSize() override { return _chrRamSize * 0x400; }

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		if(page >= _firstRamBank && page <= _lastRamBank) {
			memoryType = ChrMemoryType::ChrRam;
			page -= _firstRamBank;
		}

		MMC3::SelectChrPage(slot, page, memoryType);
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_firstRamBank);
		SV(_lastRamBank);
		SV(_chrRamSize);
	}

public:
	MMC3_ChrRam(uint16_t firstRamBank, uint16_t lastRamBank, uint16_t chrRamSize) : _firstRamBank(firstRamBank), _lastRamBank(lastRamBank), _chrRamSize(chrRamSize)
	{
	}
};
