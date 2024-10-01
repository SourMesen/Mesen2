#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"

class Smb2j : public BaseMapper
{
private:
	uint16_t _irqCounter = 0;
	bool _irqEnabled = false;

protected:
	uint16_t GetPrgPageSize() override { return 0x1000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x4122; }
	uint16_t RegisterEndAddress() override { return 0x4122; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		SelectPrgPage4x(0, 0);
		SelectPrgPage4x(1, 4);
		SelectChrPage(0, 0);

		if(_prgSize >= 0x10000) {
			AddRegisterRange(0x4022, 0x4022, MemoryOperation::Write);
		}

		SetCpuMemoryMapping(0x5000, 0x5FFF, GetPrgPageCount() - 3, PrgMemoryType::PrgRom);
		SetCpuMemoryMapping(0x6000, 0x6FFF, GetPrgPageCount() - 2, PrgMemoryType::PrgRom);
		SetCpuMemoryMapping(0x7000, 0x7FFF, GetPrgPageCount() - 1, PrgMemoryType::PrgRom);

		_irqCounter = 0;
		_irqEnabled = false;
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_irqCounter);
		SV(_irqEnabled);
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqEnabled) {
			_irqCounter = (_irqCounter + 1) & 0xFFF;
			if(_irqCounter == 0) {
				_irqEnabled = false;
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr == 0x4022) {
			SelectPrgPage4x(0, (value & 0x01) << 2);
			SelectPrgPage4x(1, ((value & 0x01) << 2) + 4);
		} else if(addr == 0x4122) {
			_irqEnabled = (value & 0x03) != 0;
			_irqCounter = 0;
			_console->GetCpu()->ClearIrqSource(IRQSource::External);
		}
	}
};