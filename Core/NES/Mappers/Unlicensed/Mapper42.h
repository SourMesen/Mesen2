#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesCpu.h"
#include "NES/NesConsole.h"

class Mapper42 : public BaseMapper
{
private:
	uint16_t _irqCounter = 0;
	bool _irqEnabled = false;
	uint8_t _prgReg = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		_irqCounter = 0;
		_irqEnabled = false;
		_prgReg = 0;

		SelectPrgPage(0, -4);
		SelectPrgPage(1, -3);
		SelectPrgPage(2, -2);
		SelectPrgPage(3, -1);
		SelectChrPage(0, 0);

		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_irqCounter);
		SV(_irqEnabled);
		SV(_prgReg);

		if(!s.IsSaving()) {
			UpdateState();
		}
	}

	void UpdateState()
	{
		SetCpuMemoryMapping(0x6000, 0x7FFF, _prgReg & 0x0F, PrgMemoryType::PrgRom);
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqEnabled) {
			_irqCounter++;
			if(_irqCounter >= 0x8000) {
				_irqCounter -= 0x8000;
			}
			if(_irqCounter >= 0x6000) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			} else {
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xE003) {
			case 0x8000:
				if(_chrRomSize > 0) {
					SelectChrPage(0, value & 0x0F);
				}
				break;

			case 0xE000:
				_prgReg = value & 0x0F;
				UpdateState();
				break;

			case 0xE001:
				SetMirroringType(value & 0x08 ? MirroringType::Horizontal : MirroringType::Vertical);
				break;

			case 0xE002:
				_irqEnabled = (value == 0x02);

				if(!_irqEnabled) {
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
					_irqCounter = 0;
				}
				break;
		}
	}
};
