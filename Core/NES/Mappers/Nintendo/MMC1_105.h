#pragma once
#include "NES/Mappers/Nintendo/MMC1.h"
#include "NES/NesCpu.h"
#include "NES/NesConsole.h"

class MMC1_105 : public MMC1
{
private:
	uint8_t _initState = 0;
	uint32_t _irqCounter = 0;
	bool _irqEnabled = false;

protected:
	uint32_t GetDipSwitchCount() override { return 4; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		MMC1::InitMapper();
		_initState = 0;
		_irqCounter = 0;
		_irqEnabled = false;
		_chrReg0 |= 0x10;  //Set I bit to 1
		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		MMC1::Serialize(s);
		SV(_initState);
		SV(_irqCounter);
		SV(_irqEnabled);
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqEnabled) {
			_irqCounter++;
			
			uint32_t maxCounter = 0x20000000 | (GetDipSwitches() << 25);
			if(_irqCounter >= maxCounter) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
				_irqEnabled = false;
			}
		}
	}

	void UpdateState() override
	{
		if(_initState == 0 && (_chrReg0 & 0x10) == 0x00) {
			_initState = 1;
		} else if(_initState == 1 && _chrReg0 & 0x10) {
			_initState = 2;
		}

		if(_chrReg0 & 0x10) {
			_irqEnabled = false;
			_irqCounter = 0;
			_console->GetCpu()->ClearIrqSource(IRQSource::External);
		} else {
			_irqEnabled = true;
		}

		MemoryAccessType access = _wramDisable ? MemoryAccessType::NoAccess : MemoryAccessType::ReadWrite;
		SetCpuMemoryMapping(0x6000, 0x7FFF, 0, HasBattery() ? PrgMemoryType::SaveRam : PrgMemoryType::WorkRam, access);

		if(_initState == 2) {
			if(_chrReg0 & 0x08) {
				//MMC1 mode
				uint8_t prgReg = (_prgReg & 0x07) | 0x08;
				if(_prgMode) {
					if(_slotSelect) {
						SelectPrgPage(0, prgReg);
						SelectPrgPage(1, 0x0F);
					} else {
						SelectPrgPage(0, 0x08);
						SelectPrgPage(1, prgReg);
					}
				} else {
					SelectPrgPage2x(0, prgReg & 0xFE);
				}
			} else {
				SelectPrgPage2x(0, _chrReg0 & 0x06);
			}
		} else {
			SelectPrgPage2x(0, 0);
		}
	}
};