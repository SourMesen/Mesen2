#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesCpu.h"
#include "NES/NesConsole.h"
#include "NES/NesMemoryManager.h"

class JyCompany : public BaseMapper
{
private:
	enum class JyIrqSource
	{
		CpuClock = 0,
		PpuA12Rise = 1,
		PpuRead = 2,
		CpuWrite = 3
	};

	uint8_t _prgRegs[4] = {};
	uint8_t _chrLowRegs[8] = {};
	uint8_t _chrHighRegs[8] = {};
	uint8_t _chrLatch[2] = {};
	
	uint8_t _prgMode = 0;
	bool _enablePrgAt6000 = false;
	
	uint8_t _chrMode = 0;
	bool _chrBlockMode = false;
	uint8_t _chrBlock = 0;
	bool _mirrorChr = false;

	uint8_t _mirroringReg = 0;
	bool _advancedNtControl = false;
	bool _disableNtRam = false;

	uint8_t _ntRamSelectBit = 0;
	uint8_t _ntLowRegs[4] = {};
	uint8_t _ntHighRegs[4] = {};

	bool _irqEnabled = false;
	JyIrqSource _irqSource = {};
	uint8_t _irqCountDirection = 0;
	bool _irqFunkyMode = false;
	uint8_t _irqFunkyModeReg = 0;
	bool _irqSmallPrescaler = false;
	uint8_t _irqPrescaler = 0;
	uint8_t _irqCounter = 0;
	uint8_t _irqXorReg = 0;

	uint8_t _multiplyValue1 = 0;
	uint8_t _multiplyValue2 = 0;
	uint8_t _regRamValue = 0;

	uint16_t _lastPpuAddr = 0;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x0400; }
	bool AllowRegisterRead() override { return true; }
	bool EnableCpuClockHook() override { return true; }
	bool EnableCustomVramRead() override { return true; }
	bool EnableVramAddressHook() override { return true; }

	void InitMapper() override
	{
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);
		AddRegisterRange(0x5000, 0x5FFF, MemoryOperation::Any);

		_chrLatch[0] = 0;
		_chrLatch[1] = 4;

		memset(_prgRegs, 0, sizeof(_prgRegs));
		memset(_chrLowRegs, 0, sizeof(_chrLowRegs));
		memset(_chrHighRegs, 0, sizeof(_chrHighRegs));

		_prgMode = 0;
		_enablePrgAt6000 = false;

		_chrMode = 0;
		_chrBlockMode = false;
		_chrBlock = 0;
		_mirrorChr = false;

		_mirroringReg = 0;
		_advancedNtControl = false;
		_disableNtRam = false;

		_ntRamSelectBit = 0;
		memset(_ntLowRegs, 0, sizeof(_ntLowRegs));
		memset(_ntHighRegs, 0, sizeof(_ntHighRegs));

		_irqEnabled = false;
		_irqSource = JyIrqSource::CpuClock;
		_lastPpuAddr = 0;
		_irqCountDirection = 0;
		_irqFunkyMode = false;
		_irqFunkyModeReg = 0;
		_irqSmallPrescaler = false;
		_irqPrescaler = 0;
		_irqCounter = 0;
		_irqXorReg = 0;

		_multiplyValue1 = 0;
		_multiplyValue2 = 0;
		_regRamValue = 0;

		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);

		SVArray(_prgRegs, 4);
		SVArray(_chrLowRegs, 8);
		SVArray(_chrHighRegs, 8);
		SVArray(_ntLowRegs, 4);
		SVArray(_ntHighRegs, 4);

		SV(_chrLatch[0]);
		SV(_chrLatch[1]);
		SV(_prgMode);
		SV(_enablePrgAt6000);
		SV(_chrMode);
		SV(_chrBlockMode);
		SV(_chrBlock);
		SV(_mirrorChr);
		SV(_mirroringReg);
		SV(_advancedNtControl);
		SV(_disableNtRam);
		SV(_ntRamSelectBit);
		SV(_irqEnabled);
		SV(_irqSource);
		SV(_lastPpuAddr);
		SV(_irqCountDirection);
		SV(_irqFunkyMode);
		SV(_irqFunkyModeReg);
		SV(_irqSmallPrescaler);
		SV(_irqPrescaler);
		SV(_irqCounter);
		SV(_irqXorReg);
		SV(_multiplyValue1);
		SV(_multiplyValue2);
		SV(_regRamValue);

		if(!s.IsSaving()) {
			UpdateState();
		}
	}

	void UpdateState()
	{
		UpdatePrgState();
		UpdateChrState();
		UpdateMirroringState();
	}

	uint8_t InvertPrgBits(uint8_t prgReg, bool needInvert)
	{
		if(needInvert) {
			return (prgReg & 0x01) << 6 | (prgReg & 0x02) << 4 | (prgReg & 0x04) << 2 | (prgReg & 0x10) >> 2 | (prgReg & 0x20) >> 4 | (prgReg & 0x40) >> 6;
		} else {
			return prgReg;
		}
	}

	void UpdatePrgState()
	{
		bool invertBits = (_prgMode & 0x03) == 0x03;
		int prgRegs[4] = { InvertPrgBits(_prgRegs[0], invertBits), InvertPrgBits(_prgRegs[1], invertBits),
								 InvertPrgBits(_prgRegs[2], invertBits), InvertPrgBits(_prgRegs[3], invertBits) };

		switch(_prgMode & 0x03) {
			case 0:
				SelectPrgPage4x(0, (_prgMode & 0x04) ? prgRegs[3] : 0x3C);
				if(_enablePrgAt6000) {
					SetCpuMemoryMapping(0x6000, 0x7FFF, prgRegs[3] * 4 + 3, PrgMemoryType::PrgRom);
				}
				break;

			case 1:
				SelectPrgPage2x(0, prgRegs[1] << 1);
				SelectPrgPage2x(1, (_prgMode & 0x04) ? prgRegs[3] : 0x3E);
				if(_enablePrgAt6000) {
					SetCpuMemoryMapping(0x6000, 0x7FFF, prgRegs[3] * 2 + 1, PrgMemoryType::PrgRom);
				}
				break;

			case 2:
			case 3:
				SelectPrgPage(0, prgRegs[0]);
				SelectPrgPage(1, prgRegs[1]);
				SelectPrgPage(2, prgRegs[2]);
				SelectPrgPage(3, (_prgMode & 0x04) ? prgRegs[3] : 0x3F);
				if(_enablePrgAt6000) {
					SetCpuMemoryMapping(0x6000, 0x7FFF, prgRegs[3], PrgMemoryType::PrgRom);
				}
				break;
		}

		if(!_enablePrgAt6000) {
			RemoveCpuMemoryMapping(0x6000, 0x7FFF);
		}
	}

	uint16_t GetChrReg(int index)
	{
		if(_chrMode >= 2 && _mirrorChr && (index == 2 || index == 3)) {
			index -= 2;
		}

		if(_chrBlockMode) {
			uint8_t mask = 0;
			uint8_t shift = 0;
			switch(_chrMode) {
				default:
				case 0: mask = 0x1F; shift = 5; break;
				case 1: mask = 0x3F; shift = 6; break;
				case 2: mask = 0x7F; shift = 7; break;
				case 3: mask = 0xFF; shift = 8; break;
			}
			return (_chrLowRegs[index] & mask) | (_chrBlock << shift);
		} else {
			return _chrLowRegs[index] | (_chrHighRegs[index] << 8);
		}
	}

	void UpdateChrState()
	{
		int chrRegs[8] = { GetChrReg(0), GetChrReg(1), GetChrReg(2), GetChrReg(3), GetChrReg(4), GetChrReg(5), GetChrReg(6), GetChrReg(7) };

		switch(_chrMode) {
			case 0: 
				SelectChrPage8x(0, chrRegs[0] << 3);
				break;

			case 1: 
				SelectChrPage4x(0, chrRegs[_chrLatch[0]] << 2);
				SelectChrPage4x(1, chrRegs[_chrLatch[1]] << 2);
				break;

			case 2: 
				SelectChrPage2x(0, chrRegs[0] << 1);
				SelectChrPage2x(1, chrRegs[2] << 1);
				SelectChrPage2x(2, chrRegs[4] << 1);
				SelectChrPage2x(3, chrRegs[6] << 1);
				break;

			case 3:
				for(int i = 0; i < 8; i++) {
					SelectChrPage(i, chrRegs[i]);
				}
				break;
		}
	}

	void UpdateMirroringState()
	{
		//"Mapper 211 behaves as though N were always set (1), and mapper 090 behaves as though N were always clear(0)."
		if((_advancedNtControl || _romInfo.MapperID == 211) && _romInfo.MapperID != 90) {
			for(int i = 0; i < 4; i++) {
				SetNametable(i, _ntLowRegs[i] & 0x01);
			}
		} else {
			switch(_mirroringReg) {
				case 0: SetMirroringType(MirroringType::Vertical); break;
				case 1: SetMirroringType(MirroringType::Horizontal); break;
				case 2: SetMirroringType(MirroringType::ScreenAOnly); break;
				case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
			}
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		switch(addr & 0xF803) {
			case 0x5000: return 0; //Dip switches
			case 0x5800: return (_multiplyValue1 * _multiplyValue2) & 0xFF;
			case 0x5801: return ((_multiplyValue1 * _multiplyValue2) >> 8) & 0xFF;
			case 0x5803: return _regRamValue;
		}

		return _console->GetMemoryManager()->GetOpenBus();
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			switch(addr & 0xF803) {
				case 0x5800: _multiplyValue1 = value; break;
				case 0x5801: _multiplyValue2 = value; break;
				case 0x5803: _regRamValue = value; break;
			}
		} else {
			switch(addr & 0xF007) {
				case 0x8000: case 0x8001: case 0x8002: case 0x8003:
				case 0x8004: case 0x8005: case 0x8006: case 0x8007:
					_prgRegs[addr & 0x03] = value & 0x7F;
					break;

				case 0x9000: case 0x9001: case 0x9002: case 0x9003:
				case 0x9004: case 0x9005: case 0x9006: case 0x9007:
					_chrLowRegs[addr & 0x07] = value;
					break;

				case 0xA000: case 0xA001: case 0xA002: case 0xA003:
				case 0xA004: case 0xA005: case 0xA006: case 0xA007:
					_chrHighRegs[addr & 0x07] = value;
					break;

				case 0xB000: case 0xB001: case 0xB002: case 0xB003:
					_ntLowRegs[addr & 0x03] = value;
					break;
				
				case 0xB004: case 0xB005: case 0xB006: case 0xB007:
					_ntHighRegs[addr & 0x03] = value;
					break;

				case 0xC000:
					if(value & 0x01) {
						_irqEnabled = true;
					} else {
						_irqEnabled = false;
						_console->GetCpu()->ClearIrqSource(IRQSource::External);
					}
					break;

				case 0xC001:
					_irqCountDirection = (value >> 6) & 0x03;
					_irqFunkyMode = (value & 0x08) == 0x08;
					_irqSmallPrescaler = ((value >> 2) & 0x01) == 0x01;
					_irqSource = (JyIrqSource)(value & 0x03);
					break;

				case 0xC002:
					_irqEnabled = false;
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
					break;

				case 0xC003: _irqEnabled = true; break;
				case 0xC004: _irqPrescaler = value ^ _irqXorReg; break;
				case 0xC005: _irqCounter = value ^ _irqXorReg; break;
				case 0xC006: _irqXorReg = value; break;
				case 0xC007: _irqFunkyModeReg = value; break;

				case 0xD000:
					_prgMode = value & 0x07;
					_chrMode = (value >> 3) & 0x03;
					_advancedNtControl = (value & 0x20) == 0x20;
					_disableNtRam = (value & 0x40) == 0x40;
					_enablePrgAt6000 = (value & 0x80) == 0x80;
					break;

				case 0xD001: _mirroringReg = value & 0x03; break;
				case 0xD002: _ntRamSelectBit = value & 0x80; break;

				case 0xD003:
					_mirrorChr = (value & 0x80) == 0x80;
					_chrBlockMode = (value & 0x20) == 0x00;
					_chrBlock = ((value & 0x18) >> 2) | (value & 0x01);
					break;

			}
		}

		UpdateState();
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqSource == JyIrqSource::CpuClock || (_irqSource == JyIrqSource::CpuWrite && _console->GetCpu()->IsCpuWrite())) {
			TickIrqCounter();
		}
	}

	uint8_t MapperReadVram(uint16_t addr, MemoryOperationType type) override
	{
		if(_irqSource == JyIrqSource::PpuRead && type == MemoryOperationType::PpuRenderingRead) {
			TickIrqCounter();
		}

		if(addr >= 0x2000) {
			//This behavior only affects reads, not writes.
			//Additional info: https://forums.nesdev.com/viewtopic.php?f=3&t=17198
			if((_advancedNtControl || _romInfo.MapperID == 211) && _romInfo.MapperID != 90) {
				uint8_t ntIndex = ((addr & 0x2FFF) - 0x2000) / 0x400;
				if(_disableNtRam || (_ntLowRegs[ntIndex] & 0x80) != (_ntRamSelectBit & 0x80)) {
					uint16_t chrPage = _ntLowRegs[ntIndex] | (_ntHighRegs[ntIndex] << 8);
					uint32_t chrOffset = chrPage * 0x400 + (addr & 0x3FF);
					if(_chrRomSize > chrOffset) {
						return _chrRom[chrOffset];
					} else {
						return 0;
					}
				}
			}
		}

		return InternalReadVram(addr);
	}

	void NotifyVramAddressChange(uint16_t addr) override
	{
		if(_irqSource == JyIrqSource::PpuA12Rise && (addr & 0x1000) && !(_lastPpuAddr & 0x1000)) {
			TickIrqCounter();
		}
		_lastPpuAddr = addr;

		if(_romInfo.MapperID == 209) {
			switch(addr & 0x2FF8) {
				case 0x0FD8:
				case 0x0FE8:
					_chrLatch[addr >> 12] = addr >> 4 & ((addr >> 10 & 0x04) | 0x02);
					UpdateChrState();
					break;
			}
		}
	}

	void TickIrqCounter()
	{
		bool clockIrqCounter = false;
		uint8_t mask = _irqSmallPrescaler ? 0x07 : 0xFF;
		uint8_t prescaler = _irqPrescaler & mask;
		if(_irqCountDirection == 0x01) {
			prescaler++;
			if((prescaler & mask) == 0) {
				clockIrqCounter = true;
			}
		} else if(_irqCountDirection == 0x02) {
			if(--prescaler == 0) {
				clockIrqCounter = true;
			}
		}
		_irqPrescaler = (_irqPrescaler & ~mask) | (prescaler & mask);

		if(clockIrqCounter) {
			if(_irqCountDirection == 0x01) {
				_irqCounter++;
				if(_irqCounter == 0 && _irqEnabled) {
					_console->GetCpu()->SetIrqSource(IRQSource::External);
				}
			} else if(_irqCountDirection == 0x02) {
				_irqCounter--;
				if(_irqCounter == 0xFF && _irqEnabled) {
					_console->GetCpu()->SetIrqSource(IRQSource::External);
				}
			}
		}
	}
};