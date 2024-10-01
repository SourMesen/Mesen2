#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"

class Mapper40 : public BaseMapper
{
private:
	uint16_t _irqCounter = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		_irqCounter = 0;

		SetCpuMemoryMapping(0x6000, 0x7FFF, 6, PrgMemoryType::PrgRom);
		SelectPrgPage(0, 4);
		SelectPrgPage(1, 5);
		SelectPrgPage(3, 7);
		SelectChrPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_irqCounter);
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqCounter > 0) {
			_irqCounter--;
			if(_irqCounter == 0) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xE000) {
			case 0x8000:
				_irqCounter = 0;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;
			case 0xA000:
				_irqCounter = 4096;
				break;
			case 0xE000:
				SelectPrgPage(2, value);
				break;
		}
	}
};
