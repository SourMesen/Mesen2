#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/Mappers/Txc/TxcChip.h"

class Txc22000 : public BaseMapper
{
private:
	TxcChip _txc = TxcChip(false);
	uint8_t _chrBank = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x8000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		AddRegisterRange(0x4100, 0x5FFF, MemoryOperation::Any);
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);

		_chrBank = 0;
		SelectPrgPage(0, 0);
		SelectChrPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_txc);
		SV(_chrBank);
	}

	void UpdateState()
	{
		SelectPrgPage(0, _txc.GetOutput() & 0x03);
		SelectChrPage(0, _chrBank);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		uint8_t openBus = _console->GetMemoryManager()->GetOpenBus();
		uint8_t value = openBus;
		if((addr & 0x103) == 0x100) {
			value = (openBus & 0xCF) | ((_txc.Read() << 4) & 0x30);
		}
		UpdateState();
		return value;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if((addr & 0xF200) == 0x4200) {
			_chrBank = value;
		}
		_txc.Write(addr, (value >> 4) & 0x03);
		UpdateState();
	}
};