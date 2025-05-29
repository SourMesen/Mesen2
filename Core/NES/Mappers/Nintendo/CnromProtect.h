#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class CnromProtect : public BaseMapper
{
private:
	bool _chrEnabled = true;

protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	bool EnableCustomVramRead() override { return true; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectChrPage(0, GetPowerOnByte());
	}

	bool HasBusConflicts() override { return true; }

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		//Submapper 0: Use heuristics - "if C AND $0F is nonzero, and if C does not equal $13: CHR is enabled"
		//Submapper 4: Enable CHR-ROM if bits 0..1 of the latch hold the value 0, otherwise disable CHR-ROM.
		//Submapper 5: Enable CHR-ROM if bits 0..1 of the latch hold the value 1, otherwise disable CHR-ROM.
		//Submapper 6: Enable CHR-ROM if bits 0..1 of the latch hold the value 2, otherwise disable CHR-ROM.
		//Submapper 7: Enable CHR-ROM if bits 0..1 of the latch hold the value 3, otherwise disable CHR-ROM.
		_chrEnabled = (
			(_romInfo.SubMapperID == 0 && (value & 0x0F) != 0 && value != 0x13) ||
			(_romInfo.SubMapperID == 4 && (value & 0x03) == 0) ||
			(_romInfo.SubMapperID == 5 && (value & 0x03) == 1) ||
			(_romInfo.SubMapperID == 6 && (value & 0x03) == 2) ||
			(_romInfo.SubMapperID == 7 && (value & 0x03) == 3)
		);

		if(_chrEnabled) {
			SelectChrPage(0, 0);
		} else {
			RemovePpuMemoryMapping(0x0000, 0x1FFF);
		}
	}

	uint8_t MapperReadVram(uint16_t addr, MemoryOperationType operationType) override
	{
		uint8_t value = InternalReadVram(addr);
		if(!_chrEnabled && addr < 0x2000) {
			//Simulate pull-up resistor on D0 when PPU reads the CHR ROM while the protection is enabled
			//This is needed for the original version of Mighty Bomb Jack to boot
			value |= 0x01;
		}
		return value;
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_chrEnabled);
	}
};