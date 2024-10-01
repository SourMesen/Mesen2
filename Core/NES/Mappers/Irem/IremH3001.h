#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"

class IremH3001 : public BaseMapper
{
private:
	bool _irqEnabled = false;
	uint16_t _irqCounter = 0;
	uint16_t _irqReloadValue = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x400; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		SelectPrgPage(0, 0);
		SelectPrgPage(1, 1);
		SelectPrgPage(2, 0xFE);
		SelectPrgPage(3, -1);
	}
	
	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_irqEnabled);
		SV(_irqCounter);
		SV(_irqReloadValue);
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqEnabled) {
			_irqCounter--;
			if(_irqCounter == 0) {
				_irqEnabled = false;
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr) {
			case 0x8000: SelectPrgPage(0, value); break;

			case 0x9001: SetMirroringType(value & 0x80 ? MirroringType::Horizontal : MirroringType::Vertical); break;
			case 0x9003: 
				_irqEnabled = (value & 0x80) == 0x80; 
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;

			case 0x9004: 
				_irqCounter = _irqReloadValue;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;

			case 0x9005: _irqReloadValue = (_irqReloadValue & 0x00FF) | (value << 8); break;
			case 0x9006: _irqReloadValue = (_irqReloadValue & 0xFF00) | value; break;

			case 0xA000: SelectPrgPage(1, value); break;
			
			case 0xB000: SelectChrPage(0, value); break;
			case 0xB001: SelectChrPage(1, value); break;
			case 0xB002: SelectChrPage(2, value); break;
			case 0xB003: SelectChrPage(3, value); break;
			case 0xB004: SelectChrPage(4, value); break;
			case 0xB005: SelectChrPage(5, value); break;
			case 0xB006: SelectChrPage(6, value); break;
			case 0xB007: SelectChrPage(7, value); break;

			case 0xC000: SelectPrgPage(2, value); break;
		}
	}
};