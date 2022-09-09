#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"
#include "NES/NesConsole.h"
#include "NES/NesMemoryManager.h"

class DragonFighter : public MMC3
{
private:
	uint8_t _exRegs[3] = {};

protected:
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		memset(_exRegs, 0, sizeof(_exRegs));
		
		MMC3::InitMapper();
		AddRegisterRange(0x6000, 0x6FFF, MemoryOperation::Any);
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SVArray(_exRegs, 3);
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		if(slot == 0) {
			SelectChrPage2x(0, ((page >> 1) ^ _exRegs[1]) << 1);
		} else if(slot == 2) {
			SelectChrPage2x(1, ((page >> 1) | ((_exRegs[2] & 0x40) << 1)) << 1);
		} else if(slot == 4) {
			SelectChrPage4x(1, (_exRegs[2] & 0x3F) << 2);
		}
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		if(slot == 0) {
			MMC3::SelectPrgPage(slot, _exRegs[0] & 0x1F);
		} else {
			MMC3::SelectPrgPage(slot, page);
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if(!(addr & 0x01)) {
			if((_exRegs[0] & 0xE0) == 0xC0) {
				_exRegs[1] = _console->GetMemoryManager()->DebugRead(0x6A);
			} else {
				_exRegs[2] = _console->GetMemoryManager()->DebugRead(0xFF);
			}
			UpdateState();
		}
		return 0;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if(!(addr & 0x01)) {
				_exRegs[0] = value;
				UpdateState();
			}
		} else {
			MMC3::WriteRegister(addr, value);
		}
	}
};