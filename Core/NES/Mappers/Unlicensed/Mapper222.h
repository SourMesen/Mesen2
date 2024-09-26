#pragma once
#include "pch.h"
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
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x400; }
	bool EnableVramAddressHook() override { return true; }

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
			case 0x8000: SelectPrgPage(0, value); break;
			case 0x9000: SetMirroringType(value & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical); break;
			case 0xA000: SelectPrgPage(1, value); break;
			case 0xB000: SelectChrPage(0, value); break;
			case 0xB002: SelectChrPage(1, value); break;
			case 0xC000: SelectChrPage(2, value); break;
			case 0xC002: SelectChrPage(3, value); break;
			case 0xD000: SelectChrPage(4, value); break;
			case 0xD002: SelectChrPage(5, value); break;
			case 0xE000: SelectChrPage(6, value); break;
			case 0xE002: SelectChrPage(7, value); break;
			case 0xF000: 
				_irqCounter = value;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;
		}
	}
};