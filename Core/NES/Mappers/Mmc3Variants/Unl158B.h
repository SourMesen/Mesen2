#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"
#include "NES/NesConsole.h"
#include "NES/NesMemoryManager.h"

class Unl158B : public MMC3
{
private:
	static constexpr uint8_t _protectionLut[8] = { 0x00, 0x00, 0x00, 0x01, 0x02, 0x04, 0x0F, 0x00 };
	uint8_t _reg = 0;

protected:
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		_reg = 0;
		AddRegisterRange(0x5000, 0x5FFF, MemoryOperation::Any);
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);
		MMC3::InitMapper();
	}

	void Reset(bool softReset) override
	{
		_reg = 0;
		ResetMmc3();
		MMC3::Reset(softReset);
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_reg);
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		if(_reg & 0x80) {
			uint32_t bank = _reg & 0x07;
			if(_reg & 0x20) {
				SelectPrgPage4x(0, (bank & 0x06) << 1);
			} else {
				SelectPrgPage2x(0, bank << 1);
				SelectPrgPage2x(1, bank << 1);
			}
		} else {
			MMC3::SelectPrgPage(slot, page & 0x0F);
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		return _console->GetMemoryManager()->GetOpenBus() | _protectionLut[addr & 0x07];
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr <= 0x5FFF) {
			if((addr & 0x07) == 0) {
				_reg = value;
				UpdatePrgMapping();
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};