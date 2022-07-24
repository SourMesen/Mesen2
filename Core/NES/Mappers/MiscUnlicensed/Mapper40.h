#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"

class Mapper40 : public BaseMapper
{
private:
	uint16_t _irqCounter = 0;

protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }

	void InitMapper() override
	{
		_irqCounter = 0;

		SetCpuMemoryMapping(0x6000, 0x7FFF, 6, PrgMemoryType::PrgRom);
		SelectPRGPage(0, 4);
		SelectPRGPage(1, 5);
		SelectPRGPage(3, 7);
		SelectCHRPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_irqCounter);
	}

	void ProcessCpuClock() override
	{
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
				SelectPRGPage(2, value);
				break;
		}
	}
};
