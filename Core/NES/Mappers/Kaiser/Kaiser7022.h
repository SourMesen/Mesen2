#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Kaiser7022 : public BaseMapper
{
private:
	uint8_t _reg = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x8000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		_reg = 0;
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);
		AddRegisterRange(0xFFFC, 0xFFFC, MemoryOperation::Any);
		SelectPrgPage(0, 0);
	}

	void Reset(bool softReset) override
	{
		_reg = 0;
		ReadRegister(0xFFFC);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_reg);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		SelectChrPage(0, _reg);
		SelectPrgPage(0, _reg);
		SelectPrgPage(1, _reg);
		
		return InternalReadRam(addr);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr) {
			case 0x8000: SetMirroringType(value & 0x04 ? MirroringType::Horizontal : MirroringType::Vertical); break;
			case 0xA000: _reg = value & 0x0F; break;
		}
	}
};