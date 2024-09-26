#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/Mappers/A12Watcher.h"
#include "NES/NesCpu.h"
#include "NES/BaseNesPpu.h"

class Mapper35 : public BaseMapper
{
private:
	uint8_t _irqCounter = 0;
	bool _irqEnabled = false;
	A12Watcher _a12Watcher;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x0400; }
	bool EnableVramAddressHook() override { return true; }

	void InitMapper() override
	{
		_irqEnabled = false;
		_irqCounter = 0;

		SelectPrgPage(3, -1);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_a12Watcher);
		SV(_irqCounter);
		SV(_irqEnabled);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xF007) {
			case 0x8000: case 0x8001: case 0x8002: case 0x8003:
				SelectPrgPage(addr & 0x03, value);
				break;

			case 0x9000: case 0x9001: case 0x9002: case 0x9003:
			case 0x9004: case 0x9005: case 0x9006: case 0x9007:
				SelectChrPage(addr & 0x07, value);
				break;

			case 0xC002: 
				_irqEnabled = false;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;

			case 0xC003: _irqEnabled = true; break;
			case 0xC005: _irqCounter = value; break;
			
			case 0xD001:
				SetMirroringType(value & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);
				break;
		}
	}
	
	void NotifyVramAddressChange(uint16_t addr) override
	{
		//MMC3-style A12 IRQ counter
		if(_a12Watcher.UpdateVramAddress(addr, _console->GetPpu()->GetFrameCycle()) == A12StateChange::Rise) {
			if(_irqEnabled) {
				_irqCounter--;
				if(_irqCounter == 0) {
					_irqEnabled = false;
					_console->GetCpu()->SetIrqSource(IRQSource::External);
				}
			}
		}
	}
};
