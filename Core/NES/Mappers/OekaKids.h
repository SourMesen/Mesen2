#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"
#include "Utilities/Serializer.h"

class OekaKids : public BaseMapper
{
	uint8_t _outerChrBank = 0;
	uint8_t _innerChrBank = 0;
	uint16_t _lastAddress = 0;

protected:
	virtual uint16_t GetPRGPageSize() override { return 0x8000; }
	virtual uint16_t GetCHRPageSize() override { return 0x1000; }
	virtual bool HasBusConflicts() override { return true; }

	void InitMapper() override
	{
		_outerChrBank = 0;
		_innerChrBank = 0;
		_lastAddress = 0;

		SelectPRGPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		s.Stream(_outerChrBank, _innerChrBank, _lastAddress);
	}

	void UpdateChrBanks()
	{
		SelectCHRPage(0, _outerChrBank | _innerChrBank);
		SelectCHRPage(1, _outerChrBank | 0x03);
	}

	void NotifyVramAddressChange(uint16_t addr) override
	{
		if((_lastAddress & 0x3000) != 0x2000 && (addr & 0x3000) == 0x2000) {
			_innerChrBank = (addr >> 8) & 0x03;
			UpdateChrBanks();
		}

		_lastAddress = addr;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		SelectPRGPage(0, value & 0x03);
		_outerChrBank = value & 0x04;
		UpdateChrBanks();
	}
};
