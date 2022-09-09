#pragma once
#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Bmc8157 : public BaseMapper
{
private:
	uint16_t _lastAddr = 0;

protected:
	uint32_t GetDipSwitchCount() override { return 1; }
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_lastAddr = 0;
		UpdateState();
		SelectChrPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_lastAddr);

		if(!s.IsSaving()) {
			UpdateState();
		}
	}

	void UpdateState()
	{
		uint8_t innerPrg0 = (_lastAddr >> 2) & 0x07;
		uint8_t innerPrg1 = ((_lastAddr >> 7) & 0x01) | ((_lastAddr >> 8) & 0x02);
		uint8_t outer128Prg = (_lastAddr >> 5) & 0x03;
		uint8_t outer512Prg = (_lastAddr >> 8) & 0x01;

		int baseBank;
		if(innerPrg1 == 0) {
			baseBank = 0;
		} else if(innerPrg1 == 1) {
			baseBank = innerPrg0;
		} else {
			baseBank = 7;
		}

		if(outer512Prg && _prgSize <= 1024 * 512 && GetDipSwitches() != 0) {
			RemoveCpuMemoryMapping(0x8000, 0xFFFF);
		} else {
			SelectPrgPage(0, (outer512Prg << 6) | (outer128Prg << 3) | innerPrg0);
			SelectPrgPage(1, (outer512Prg << 6) | (outer128Prg << 3) | baseBank);
			SetMirroringType(_lastAddr & 0x02 ? MirroringType::Horizontal : MirroringType::Vertical);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_lastAddr = addr;
		UpdateState();
	}
};