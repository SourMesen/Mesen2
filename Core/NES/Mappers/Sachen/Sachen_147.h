#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "NES/NesMemoryManager.h"
#include "NES/Mappers/Txc/TxcChip.h"

class Sachen_147 : public BaseMapper
{
private:
	TxcChip _txc = TxcChip(true);

protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x8000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		AddRegisterRange(0x4020, 0x5FFF, MemoryOperation::Any);
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);

		SelectPrgPage(0, 0);
		SelectChrPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_txc);
	}

	void UpdateState()
	{
		uint8_t out = _txc.GetOutput();
		SelectPrgPage(0, ((out & 0x20) >> 4) | (out & 0x01));
		SelectChrPage(0, (out & 0x1E) >> 1);
	}
	
	uint8_t ReadRegister(uint16_t addr) override
	{
		uint8_t openBus = _console->GetMemoryManager()->GetOpenBus();
		uint8_t value = openBus;
		if((addr & 0x103) == 0x100) {
			uint8_t v = _txc.Read();
			value = ((v & 0x3F) << 2) | ((v & 0xC0) >> 6);
		}
		UpdateState();
		return value;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_txc.Write(addr, ((value & 0xFC) >> 2) | ((value & 0x03) << 6));
		if(addr >= 0x8000) {
			UpdateState();
		}
	}
};