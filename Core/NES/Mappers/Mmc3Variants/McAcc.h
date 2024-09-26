#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

//Based on krikzz's research: https://forums.nesdev.com/viewtopic.php?p=242427#p242427
class McAcc : public MMC3
{
private:
	uint32_t _counter = 0;
	uint16_t _prevAddr = 0;

protected:
	bool EnableVramAddressHook() override { return true; }

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SV(_counter);
		SV(_prevAddr);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if((addr & 0xE001) == 0xC001) {
			//"Writing to $C001 resets pulse counter."
			_counter = 0;
		}
		MMC3::WriteRegister(addr, value);
	}

	void NotifyVramAddressChange(uint16_t addr) override
	{
		if(!(addr & 0x1000) && (_prevAddr & 0x1000)) {
			_counter++;

			if(_counter == 1) {
				//"Counter clocking happens once per 8 A12 cycles at first cycle"
				if(_irqCounter == 0 || _irqReload) {
					_irqCounter = _irqReloadValue;
				} else {
					_irqCounter--;
				}

				if(_irqCounter == 0 && _irqEnabled) {
					_console->GetCpu()->SetIrqSource(IRQSource::External);
				}

				_irqReload = false;
			} else if(_counter == 8) {
				_counter = 0;
			}
		}
		_prevAddr = addr;
	}
};