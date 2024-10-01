#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesCpu.h"
#include "NES/NesConsole.h"

class Mapper106 : public BaseMapper
{
private:
	uint16_t _irqCounter = 0;
	bool _irqEnabled = false;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x400; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		_irqEnabled = false;
		_irqCounter = 0;

		SelectPrgPage(0, -1);
		SelectPrgPage(1, -1);
		SelectPrgPage(2, -1);
		SelectPrgPage(3, -1);
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
			if(_irqCounter == 0) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
				_irqEnabled = false;
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0x0F) {
			case 0: case 2: SelectChrPage(addr & 0x0F, value & 0xFE); break;
			case 1: case 3: SelectChrPage(addr & 0x0F, value | 0x01); break;
			case 4: case 5: case 6: case 7: SelectChrPage(addr & 0x0F, value); break;

			case 8: case 0x0B: SelectPrgPage((addr & 0x0F) - 8, (value & 0x0F) | 0x10); break;
			case 9: case 0x0A: SelectPrgPage((addr & 0x0F) - 8, value & 0x1F); break;

			case 0x0D: 
				_irqEnabled = false; 
				_irqCounter = 0; 
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;

			case 0x0E:
				_irqCounter = (_irqCounter & 0xFF00) | value;
				break;

			case 0x0F:
				_irqCounter = (_irqCounter & 0xFF) | (value << 8);
				_irqEnabled = true;
				break;
		}
	}
};