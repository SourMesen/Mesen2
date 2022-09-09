#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/Mappers/Txc/TxcChip.h"

class Txc22211B : public BaseMapper
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
		SelectChrPage(0, _txc.GetOutput());
		SetMirroringType(_txc.GetInvertFlag() ? MirroringType::Vertical : MirroringType::Horizontal);
	}

	uint8_t ConvertValue(uint8_t v)
	{
		return ((v & 0x01) << 5) | ((v & 0x02) << 3) | ((v & 0x04) << 1) | ((v & 0x08) >> 1) | ((v & 0x10) >> 3) | ((v & 0x20) >> 5);
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		uint8_t openBus = _console->GetMemoryManager()->GetOpenBus();
		uint8_t value = openBus;
		if((addr & 0x103) == 0x100) {
			value = (openBus & 0xC0) | ConvertValue(_txc.Read());
		}
		UpdateState();
		return value;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		_txc.Write(addr, ConvertValue(value));
		if(addr >= 0x8000) {
			UpdateState();
		}
	}
};