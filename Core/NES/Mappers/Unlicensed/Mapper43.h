#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"

class Mapper43 : public BaseMapper
{
private:
	uint8_t _reg = 0;
	bool _swap = false;
	uint16_t _irqCounter = 0;
	bool _irqEnabled = false;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint16_t RegisterStartAddress() override { return 0x4020; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		_irqCounter = 0;
		_irqEnabled = false;
		_swap = false;
		_reg = 0;

		UpdateState();
		SetCpuMemoryMapping(0x5000, 0x5FFF, 8, PrgMemoryType::PrgRom);
		SelectPrgPage(0, 1);
		SelectPrgPage(1, 0);
		SelectChrPage(0, 0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_irqCounter);
		SV(_irqEnabled);
		SV(_reg);
		SV(_swap);

		if(!s.IsSaving()) {
			UpdateState();
		}
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqEnabled) {
			_irqCounter++;
			if(_irqCounter >= 4096) {
				_irqEnabled = false;
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
		}
	}

	void UpdateState()
	{
		SetCpuMemoryMapping(0x6000, 0x7FFF, _swap ? 0 : 2, PrgMemoryType::PrgRom);
		SelectPrgPage(2, _reg);
		SelectPrgPage(3, _swap ? 8 : 9);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		int lut[8] = { 4, 3, 5, 3, 6, 3, 7, 3 };
		switch(addr & 0xF1FF) {
			case 0x4022: _reg = lut[value & 0x07]; UpdateState(); break;
			case 0x4120: _swap = value & 0x01; UpdateState(); break;
			
			case 0x8122:
			case 0x4122:
				_irqEnabled = (value & 0x01) == 0x01;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				_irqCounter = 0;
				break;
		}
	}
};
