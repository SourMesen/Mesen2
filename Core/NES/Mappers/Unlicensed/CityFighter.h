#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesMemoryManager.h"
#include "NES/NesCpu.h"
#include "NES/NesConsole.h"

class CityFighter : public BaseMapper
{
private:
	uint8_t _prgReg = 0;
	uint8_t _prgMode = 0;
	uint8_t _mirroring = 0;
	uint8_t _chrRegs[8] = {};
	bool _irqEnabled = false;
	uint16_t _irqCounter = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x400; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		_prgReg = 0;
		_prgMode = 0;
		_mirroring = 0;
		_irqCounter = 0;
		_irqEnabled = false;
		memset(_chrRegs, 0, sizeof(_chrRegs));

		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_chrRegs, 8);
		SV(_prgReg);
		SV(_prgMode);
		SV(_mirroring);
		SV(_irqEnabled);
		SV(_irqCounter);
	}

	void UpdateState()
	{
		SelectPrgPage4x(0x8000, _prgReg);
		if(!_prgMode) {
			SelectPrgPage(2, _prgReg);
		}

		for(int i = 0; i < 8; i++) {
			SelectChrPage(i, _chrRegs[i]);
		}

		switch(_mirroring) {
			case 0: SetMirroringType(MirroringType::Vertical); break;
			case 1: SetMirroringType(MirroringType::Horizontal); break;
			case 2: SetMirroringType(MirroringType::ScreenAOnly); break;
			case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
		}
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqEnabled) {
			_irqCounter--;
			if(_irqCounter == 0) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xF00C) {
			case 0x9000:
				_prgReg = value & 0x0C;
				_mirroring = value & 0x03;
				break;
			
			case 0x9004: case 0x9008: case 0x900C:
				if(addr & 0x800) {
					_console->GetMemoryManager()->Write(0x4011, (value & 0x0F) << 3, MemoryOperationType::Write);
				} else {
					_prgReg = value & 0x0C;
				}
				break;

			case 0xC000: case 0xC004: case 0xC008: case 0xC00C:
				_prgMode = value & 0x01;
				break;

			case 0xD000: _chrRegs[0] = (_chrRegs[0] & 0xF0) | (value & 0x0F); break;
			case 0xD004: _chrRegs[0] = (_chrRegs[0] & 0x0F) | (value << 4); break;
			case 0xD008: _chrRegs[1] = (_chrRegs[1] & 0xF0) | (value & 0x0F); break;
			case 0xD00C: _chrRegs[1] = (_chrRegs[1] & 0x0F) | (value << 4); break;
			case 0xA000: _chrRegs[2] = (_chrRegs[2] & 0xF0) | (value & 0x0F); break;
			case 0xA004: _chrRegs[2] = (_chrRegs[2] & 0x0F) | (value << 4); break;
			case 0xA008: _chrRegs[3] = (_chrRegs[3] & 0xF0) | (value & 0x0F); break;
			case 0xA00C: _chrRegs[3] = (_chrRegs[3] & 0x0F) | (value << 4); break;
			case 0xB000: _chrRegs[4] = (_chrRegs[4] & 0xF0) | (value & 0x0F); break;
			case 0xB004: _chrRegs[4] = (_chrRegs[4] & 0x0F) | (value << 4); break;
			case 0xB008: _chrRegs[5] = (_chrRegs[5] & 0xF0) | (value & 0x0F); break;
			case 0xB00C: _chrRegs[5] = (_chrRegs[5] & 0x0F) | (value << 4); break;
			case 0xE000: _chrRegs[6] = (_chrRegs[6] & 0xF0) | (value & 0x0F); break;
			case 0xE004: _chrRegs[6] = (_chrRegs[6] & 0x0F) | (value << 4); break;
			case 0xE008: _chrRegs[7] = (_chrRegs[7] & 0xF0) | (value & 0x0F); break;
			case 0xE00C: _chrRegs[7] = (_chrRegs[7] & 0x0F) | (value << 4); break;
			case 0xF000: _irqCounter = ((_irqCounter & 0x1E0) | ((value & 0x0F) << 1)); break;
			case 0xF004: _irqCounter = ((_irqCounter & 0x1E) | ((value & 0x0F) << 5)); break;
			case 0xF008:
				_irqEnabled = (value & 0x02) != 0;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;
		}

		UpdateState();
	}
};