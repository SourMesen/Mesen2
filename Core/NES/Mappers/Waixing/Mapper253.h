#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class Mapper253 : public BaseMapper
{
private:
	uint8_t _chrLow[8] = {};
	uint8_t _chrHigh[8] = {};
	bool _forceChrRom = false;
	uint8_t _irqReloadValue = 0;
	uint8_t _irqCounter = 0;
	bool _irqEnabled = false;
	uint16_t _irqScaler = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x400; }
	uint32_t GetChrRamSize() override { return 0x800; }
	uint16_t GetChrRamPageSize() override { return 0x400; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		memset(_chrLow, 0, sizeof(_chrLow));
		memset(_chrHigh, 0, sizeof(_chrHigh));
		_forceChrRom = false;
		_irqReloadValue = 0;
		_irqScaler = 0;
		_irqCounter = 0;
		_irqEnabled = false;

		SelectPrgPage(2, -2);
		SelectPrgPage(3, -1);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);

		SVArray(_chrLow, 8);
		SVArray(_chrHigh, 8);
		SV(_forceChrRom);
		SV(_irqReloadValue);
		SV(_irqCounter);
		SV(_irqEnabled);
		SV(_irqScaler);

		if(!s.IsSaving()) {
			UpdateChr();
		}
	}

	void UpdateChr()
	{
		for(uint16_t i = 0; i < 8; i++) {
			uint16_t page = _chrLow[i] | (_chrHigh[i] << 8);
			if((_chrLow[i] == 4 || _chrLow[i] == 5) && !_forceChrRom) {
				SelectChrPage(i, page & 0x01, ChrMemoryType::ChrRam);
			} else {
				SelectChrPage(i, page);
			}
		}
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqEnabled) {
			_irqScaler++;
			if(_irqScaler >= 114) {
				_irqScaler = 0;
				_irqCounter++;
				if(_irqCounter == 0) {
					_irqCounter = _irqReloadValue;
					_console->GetCpu()->SetIrqSource(IRQSource::External);
				}
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr >= 0xB000 && addr <= 0xE00C) {
			uint8_t slot = ((((addr & 0x08) | (addr >> 8)) >> 3) + 2) & 0x07;
			uint8_t shift = addr & 0x04;
			uint8_t chrLow = (_chrLow[slot] & (0xF0 >> shift)) | (value << shift);
			_chrLow[slot] = chrLow;
			if(slot == 0) {
				if(chrLow == 0xc8) {
					_forceChrRom = false;
				} else if(chrLow == 0x88) {
					_forceChrRom = true;
				}
			}
			if(shift) {
				_chrHigh[slot] = value >> 4;
			}
			UpdateChr();
		} else {
			switch(addr) {
				case 0x8010: SelectPrgPage(0, value); break;
				case 0xA010: SelectPrgPage(1, value); break;
				case 0x9400:
					switch(value & 0x03) {
						case 0: SetMirroringType(MirroringType::Vertical); break;
						case 1: SetMirroringType(MirroringType::Horizontal); break;
						case 2: SetMirroringType(MirroringType::ScreenAOnly); break;
						case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
					}
					break;

				case 0xF000:
					_irqReloadValue = (_irqReloadValue & 0xF0) | (value & 0x0F);
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
					break;

				case 0xF004:
					_irqReloadValue = (_irqReloadValue & 0x0F) | (value << 4);
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
					break;

				case 0xF008:
					_irqCounter = _irqReloadValue;
					_irqEnabled = (value & 0x02) == 0x02;
					_irqScaler = 0;
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
					break;
			}
		}
	}
};