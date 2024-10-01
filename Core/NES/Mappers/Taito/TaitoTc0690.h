#pragma once
#include "pch.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class TaitoTc0690 : public MMC3
{
private:
	uint8_t _irqDelay = 0;
	bool _isFlintstones = false;

protected:
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		_irqDelay = 0;
		SelectPrgPage(2, -2);
		SelectPrgPage(3, -1);

		//This cart appears to behave differently (maybe not an identical mapper?)
		//IRQ seems to be triggered at a different timing (approx 100 cpu cycles before regular mapper 48 timings)
		_isFlintstones = _romInfo.SubMapperID == 255;
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_irqDelay);
	}

	void TriggerIrq() override
	{
		//"The IRQ seems to trip a little later than it does on MMC3.  It looks like about a 4 CPU cycle delay from the normal MMC3 IRQ time."
		//A value of 6 removes the shaking from The Jetsons
		_irqDelay = _isFlintstones ? 19 : 6;
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqDelay > 0) {
			_irqDelay--;
			if(_irqDelay == 0) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xE003) {
			case 0x8000:
				SelectPrgPage(0, value & 0x3F);
				break;
			case 0x8001:
				SelectPrgPage(1, value & 0x3F);
				break;
			case 0x8002:
				SelectChrPage(0, value * 2);
				SelectChrPage(1, value * 2 + 1);
				break;
			case 0x8003:
				SelectChrPage(2, value * 2);
				SelectChrPage(3, value * 2 + 1);
				break;
			case 0xA000: case 0xA001: case 0xA002: case 0xA003:
				SelectChrPage(4 + (addr & 0x03), value);
				break;

			case 0xC000:
				//Flintstones expects either $C000 or $C001 to clear the irq flag
				_console->GetCpu()->ClearIrqSource(IRQSource::External);

				_irqReloadValue = (value ^ 0xFF) + (_isFlintstones ? 0 : 1);
				break;
			case 0xC001:
				//Flintstones expects either $C000 or $C001 to clear the irq flag
				_console->GetCpu()->ClearIrqSource(IRQSource::External);

				_irqCounter = 0;
				_irqReload = true;
				break;
			case 0xC002:
				_irqEnabled = true;
				break;
			case 0xC003:
				_irqEnabled = false;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;

			case 0xE000:
				SetMirroringType((value & 0x40) == 0x40 ? MirroringType::Horizontal : MirroringType::Vertical);
				break;
		}
	}
};