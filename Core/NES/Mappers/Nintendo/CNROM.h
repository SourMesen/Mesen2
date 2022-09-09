#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class CNROM : public BaseMapper
{
private:
	bool _enableCopyProtection = false;

protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectChrPage(0, GetPowerOnByte());
	}
	
	bool HasBusConflicts() override { return (_romInfo.MapperID == 3 && _romInfo.SubMapperID == 2) || _romInfo.MapperID == 185; }

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(_enableCopyProtection) {
			//Submapper 0: Use heuristics - "if C AND $0F is nonzero, and if C does not equal $13: CHR is enabled"
			//Submapper 4: Enable CHR-ROM if bits 0..1 of the latch hold the value 0, otherwise disable CHR-ROM.
			//Submapper 5: Enable CHR-ROM if bits 0..1 of the latch hold the value 1, otherwise disable CHR-ROM.
			//Submapper 6: Enable CHR-ROM if bits 0..1 of the latch hold the value 2, otherwise disable CHR-ROM.
			//Submapper 7: Enable CHR-ROM if bits 0..1 of the latch hold the value 3, otherwise disable CHR-ROM.
			bool validAccess = (
				(_romInfo.SubMapperID == 0 && (value & 0x0F) != 0 && value != 0x13) ||
				(_romInfo.SubMapperID == 4 && (value & 0x03) == 0) ||
				(_romInfo.SubMapperID == 5 && (value & 0x03) == 1) ||
				(_romInfo.SubMapperID == 6 && (value & 0x03) == 2) ||
				(_romInfo.SubMapperID == 7 && (value & 0x03) == 3)
			);

			if(validAccess) {
				SelectChrPage(0, 0);
			} else {
				RemovePpuMemoryMapping(0x0000, 0x1FFF);
			}
		} else {
			SelectChrPage(0, value);
		}
	}

public:
	CNROM(bool enableCopyProtection) : _enableCopyProtection(enableCopyProtection)
	{
	}
};