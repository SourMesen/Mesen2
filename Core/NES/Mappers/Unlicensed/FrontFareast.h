#pragma once
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "NES/NesCpu.h"

class FrontFareast : public BaseMapper
{
private:
	uint16_t _irqCounter = 0;
	bool _irqEnabled = false;
	bool _ffeAltMode = false;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x400; }
	uint32_t GetChrRamSize() override { return 0x8000; }
	uint16_t RegisterStartAddress() override { return 0x42FE; }
	uint16_t RegisterEndAddress() override { return 0x4517; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		_irqCounter = 0;
		_irqEnabled = false;
		_ffeAltMode = true;

		switch(_romInfo.MapperID) {
			case 6:
				AddRegisterRange(0x8000, 0xFFFF, MemoryOperation::Write);
				SelectPrgPage2x(0, 0);
				SelectPrgPage2x(1, 14);
				break;

			case 8:
				AddRegisterRange(0x8000, 0xFFFF, MemoryOperation::Write);
				SelectPrgPage4x(0, 0);
				break;

			case 17:
				SelectPrgPage4x(0, -4);
				break;
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_irqCounter);
		SV(_irqEnabled);
		SV(_ffeAltMode);
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqEnabled) {
			_irqCounter++;
			if(_irqCounter == 0) {
				_console->GetCpu()->SetIrqSource(IRQSource::External);
				_irqEnabled = false;
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr) {
			case 0x42FE:
				_ffeAltMode = (value & 0x80) == 0x00;
				switch((value >> 4) & 0x01) {
					case 0: SetMirroringType(MirroringType::ScreenAOnly); break;
					case 1: SetMirroringType(MirroringType::ScreenBOnly); break;
				}
				break;

			case 0x42FF:
				switch((value >> 4) & 0x01) {
					case 0: SetMirroringType(MirroringType::Vertical); break;
					case 1: SetMirroringType(MirroringType::Horizontal); break;
				}
				break;

			case 0x4501: 
				_irqEnabled = false;
				_console->GetCpu()->ClearIrqSource(IRQSource::External); 
				break;

			case 0x4502: 
				_irqCounter = (_irqCounter & 0xFF00) | value;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;

			case 0x4503: 
				_irqCounter = (_irqCounter & 0x00FF) | (value << 8);
				_irqEnabled = true;
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				break;

			default:
				if(_romInfo.MapperID == 6) {
					if(addr >= 0x8000) {
						if(HasChrRam() || _ffeAltMode) {
							SelectPrgPage2x(0, (value & 0xFC) >> 1);
							value &= 0x03;
						}
						SelectChrPage8x(0, value << 3);
					}
				} else if(_romInfo.MapperID == 8) {
					if(addr >= 0x8000) {
						SelectPrgPage2x(0, (value & 0xF8) >> 2);
						SelectChrPage8x(0, (value & 0x07) << 3);
					}
				} else {
					switch(addr) {
						case 0x4504: case 0x4505: case 0x4506: case 0x4507:
							SelectPrgPage(addr - 0x4504, value);
							break;

						case 0x4510: case 0x4511: case 0x4512: case 0x4513: case 0x4514: case 0x4515: case 0x4516: case 0x4517:
							SelectChrPage(addr - 0x4510, value);
							break;
					}
				}
		}
	}
};
