#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"

class Mapper50 : public BaseMapper
{
private:
	uint16_t _irqCounter = 0;
	bool _irqEnabled = false;

protected:
	uint16_t RegisterStartAddress() override { return 0x4020; }
	uint16_t RegisterEndAddress() override { return 0x5FFF; }
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		_irqCounter = 0;
		_irqEnabled = false;

		SetCpuMemoryMapping(0x6000, 0x7FFF, 0x0F, PrgMemoryType::PrgRom);
		SelectPrgPage(0, 0x08);
		SelectPrgPage(1, 0x09);
		SelectPrgPage(3, 0x0B);
		SelectChrPage(0, 0);
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
			_irqCounter++;
			if(_irqCounter == 0x1000) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
				_irqEnabled = false;
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0x4120) {
			case 0x4020:
				SelectPrgPage(2, (value & 0x08) | ((value & 0x01) << 2) | ((value & 0x06) >> 1));
				break;

			case 0x4120:
				if(value & 0x01) {
					_irqEnabled = true;
				} else {
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
					_irqCounter = 0;
					_irqEnabled = false;
				}
				break;
		}
	}
};
