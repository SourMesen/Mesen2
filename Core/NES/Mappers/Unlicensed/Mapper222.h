#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"
#include "NES/NesCpu.h"
#include "NES/NesConsole.h"
#include "NES/Mappers/A12Watcher.h"

class Mapper222 : public BaseMapper
{
private:
	uint16_t _irqCounter = 0;
	A12Watcher _a12Watcher;

protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x400; }

	void InitMapper() override
	{
		_irqCounter = 0;

		SelectPrgPage2x(1, -2);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_a12Watcher);
		SV(_irqCounter);
	}

	void NotifyVramAddressChange(uint16_t addr) override
	{
		if(_a12Watcher.UpdateVramAddress(addr, _console->GetPpu()->GetFrameCycle()) == A12StateChange::Rise) {
			if(_irqCounter) {
				_irqCounter++;
				if(_irqCounter >= 240) {
					_console->GetCpu()->SetIrqSource(IRQSource::External);
					_irqCounter = 0;
				}
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xF003) {
			case 0x8000: SelectPRGPage(0, value); break;
			case 0x9000: SetMirroringType(value & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical); break;
			case 0xA000: SelectPRGPage(1, value); break;
			case 0xB000: SelectCHRPage(0, value); break;
			case 0xB002: SelectCHRPage(1, value); break;
			case 0xC000: SelectCHRPage(2, value); break;
			case 0xC002: SelectCHRPage(3, value); break;
			case 0xD000: SelectCHRPage(4, value); break;
			case 0xD002: SelectCHRPage(5, value); break;
			case 0xE000: SelectCHRPage(6, value); break;
			case 0xE002: SelectCHRPage(7, value); break;
			case 0xF000: 
				_irqCounter = value;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;
		}
	}
};