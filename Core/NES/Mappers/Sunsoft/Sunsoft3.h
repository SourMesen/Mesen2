#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesCpu.h"
#include "NES/NesConsole.h"

class Sunsoft3 : public BaseMapper
{
private:
	bool _irqLatch = false;
	bool _irqEnabled = false;
	uint16_t _irqCounter = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x800; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		SelectPrgPage(1, -1);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_irqLatch);
		SV(_irqEnabled);
		SV(_irqCounter);
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqEnabled) {
			_irqCounter--;
			if(_irqCounter == 0xFFFF) {
				_irqEnabled = false;
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xF800) {
			case 0x8800: SelectChrPage(0, value); break;
			case 0x9800: SelectChrPage(1, value); break;
			case 0xA800: SelectChrPage(2, value); break;
			case 0xB800: SelectChrPage(3, value); break;
			case 0xC800: 
				_irqCounter &= _irqLatch ? 0xFF00 : 0x00FF;
				_irqCounter |= _irqLatch ? value : (value << 8);
				_irqLatch = !_irqLatch;
				break;
			case 0xD800:
				_irqEnabled = (value & 0x10) == 0x10;
				_irqLatch = false;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;
			case 0xE800:
				switch(value & 0x03) {
					case 0: SetMirroringType(MirroringType::Vertical); break;
					case 1: SetMirroringType(MirroringType::Horizontal); break;
					case 2: SetMirroringType(MirroringType::ScreenAOnly); break;
					case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
				}
				break;
			case 0xF800: SelectPrgPage(0, value); break;
		}
	}
};
