#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Racermate : public BaseMapper
{
private:
	uint16_t _irqCounter = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x1000; }
	uint32_t GetChrRamSize() override { return 0x10000; }
	uint32_t GetSaveRamSize() override { return 0; }
	bool ForceChrBattery() override { return true; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		_irqCounter = 0;

		SelectPrgPage(1, -1);
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

		_irqCounter--;
		if(_irqCounter == 0) {
			_irqCounter = 1024;
			_console->GetCpu()->SetIrqSource(IRQSource::External);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xC000) {
			case 0x8000: 
				SelectPrgPage(0, (value >> 6) & 0x03);
				SelectChrPage(1, value & 0x0F);
				break;

			case 0xC000: 
				_irqCounter = 1024;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;
		}

	}
};