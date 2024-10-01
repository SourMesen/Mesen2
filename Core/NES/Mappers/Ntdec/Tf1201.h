#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Tf1201 : public BaseMapper
{
private:
	uint8_t _chrRegs[8] = {};
	uint8_t _prgRegs[2] = {};
	bool _swapPrg = false;
	uint8_t _irqCounter = 0;
	uint8_t _irqReloadValue = 0;
	int16_t _irqScaler = 0;
	bool _irqEnabled = false;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x400; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		memset(_prgRegs, 0, sizeof(_prgRegs));
		memset(_chrRegs, 0, sizeof(_chrRegs));
		_swapPrg = false;

		_irqCounter = 0;
		_irqReloadValue = 0;
		_irqScaler = 0;
		_irqEnabled = false;

		UpdateChr();
		UpdatePrg();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_prgRegs, 2);
		SVArray(_chrRegs, 8);
		SV(_swapPrg);
		SV(_irqCounter);
		SV(_irqReloadValue);
		SV(_irqScaler);
		SV(_irqEnabled);
	}

	void UpdateChr()
	{
		for(int i = 0; i < 8; i++) {
			SelectChrPage(i, _chrRegs[i]);
		}
	}

	void UpdatePrg()
	{
		if(_swapPrg) {
			SelectPrgPage(0, -2);
			SelectPrgPage(2, _prgRegs[0]);
		} else {
			SelectPrgPage(0, _prgRegs[0]);
			SelectPrgPage(2, -2);
		}
		SelectPrgPage(1, _prgRegs[1]);
		SelectPrgPage(3, -1);
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqEnabled) {
			_irqScaler -= 3;
			if(_irqScaler <= 0) {
				_irqScaler += 341;
				_irqCounter++;
				if(_irqCounter == 0) {
					_console->GetCpu()->SetIrqSource(IRQSource::External);
				}
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		addr = (addr & 0xF003) | ((addr & 0x0C) >> 2);
		if(addr >= 0xB000 && addr <= 0xE003) {
			int slot = (((addr >> 11) - 6) | (addr & 0x01)) & 0x07;
			int shift = ((addr & 0x02) << 1);
			_chrRegs[slot] = (_chrRegs[slot] & (0xF0 >> shift)) | ((value & 0x0F) << shift);
			UpdateChr();
		} else {
			switch(addr & 0xF003) {
				case 0x8000: _prgRegs[0] = value; UpdatePrg(); break;
				case 0xA000: _prgRegs[1] = value; UpdatePrg(); break;
				case 0x9000: SetMirroringType(value & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical); break;
				case 0x9001: _swapPrg = (value & 0x03) != 0; UpdatePrg(); break;

				case 0xF000: _irqReloadValue = (_irqReloadValue & 0xF0) | (value & 0x0F); break;
				case 0xF002: _irqReloadValue = (_irqReloadValue & 0x0F) | (value << 4); break;
				
				case 0xF001:
					//VRC-like IRQs?  This seems to make more sense than A12-based IRQs
					//considering FCEUX/puNES both adjust the counter value based on when
					//on the screen the IRQ is enabled.
					//This still some glitches on the screen, but relatively minor ones.
					_irqEnabled = (value & 0x02) == 0x02;
					if(_irqEnabled) {
						_irqScaler = 341;
						_irqCounter = _irqReloadValue;
					}
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
					break;

				case 0xF003: 
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
					break;
			}
		}
	}
};