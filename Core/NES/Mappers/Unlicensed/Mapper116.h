#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesCpu.h"
#include "NES/Mappers/A12Watcher.h"

class Mapper116 : public BaseMapper
{
private:
	A12Watcher _a12Watcher;
	uint8_t _mode = 0;

	uint8_t _vrc2Chr[8] = {};
	uint8_t _vrc2Prg[2] = {};
	uint8_t _vrc2Mirroring = 0;

	uint8_t _mmc3Regs[10] = {};
	uint8_t _mmc3Ctrl = 0;
	uint8_t _mmc3Mirroring = 0;

	uint8_t _mmc1Regs[4] = {};
	uint8_t _mmc1Buffer = 0;
	uint8_t _mmc1Shift = 0;

	uint8_t _irqCounter = 0;
	uint8_t _irqReloadValue = 0;
	bool _irqReload = false;
	bool _irqEnabled = false;

protected:
	uint16_t RegisterStartAddress() override { return 0x4100; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x400; }
	bool EnableVramAddressHook() override { return true; }

	void InitMapper() override
	{
		_mode = 0;

		_vrc2Chr[0] = -1;
		_vrc2Chr[1] = -1;
		_vrc2Chr[2] = -1;
		_vrc2Chr[3] = -1;
		_vrc2Chr[4] = 4;
		_vrc2Chr[5] = 5;
		_vrc2Chr[6] = 6;
		_vrc2Chr[7] = 7;
		_vrc2Prg[0] = 0;
		_vrc2Prg[1] = 1;
		_vrc2Mirroring = 0;
		
		_mmc3Regs[0] = 0;
		_mmc3Regs[1] = 2;
		_mmc3Regs[2] = 4;
		_mmc3Regs[3] = 5;
		_mmc3Regs[4] = 6;
		_mmc3Regs[5] = 7;
		_mmc3Regs[6] = -4;
		_mmc3Regs[7] = -3;
		_mmc3Regs[8] = -2;
		_mmc3Regs[9] = -1;
		_mmc3Ctrl = 0;
		_mmc3Mirroring = 0;
		_irqCounter = 0;
		_irqReloadValue = 0;
		_irqEnabled = false;
		_irqReload = false;

		_mmc1Regs[0] = 0xc;
		_mmc1Regs[1] = 0;
		_mmc1Regs[2] = 0;
		_mmc1Regs[3] = 0;
		_mmc1Buffer = 0;
		_mmc1Shift = 0;

		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);

		SV(_a12Watcher);
		SVArray(_vrc2Chr, 8);
		SVArray(_vrc2Prg, 2);
		SVArray(_mmc3Regs, 10);
		SVArray(_mmc1Regs, 4);

		SV(_mode);
		SV(_vrc2Mirroring);
		SV(_mmc3Ctrl);
		SV(_mmc3Mirroring);
		SV(_irqCounter);
		SV(_irqEnabled);
		SV(_irqReload);
		SV(_irqReloadValue),
		SV(_mmc1Buffer);
		SV(_mmc1Shift);
	}

	void NotifyVramAddressChange(uint16_t addr) override
	{
		if((_mode & 0x03) == 1) {
			switch(_a12Watcher.UpdateVramAddress(addr, _console->GetPpu()->GetFrameCycle())) {
				case A12StateChange::None:
				case A12StateChange::Fall:
					break;

				case A12StateChange::Rise:
					if(_irqCounter == 0 || _irqReload) {
						_irqCounter = _irqReloadValue;
					} else {
						_irqCounter--;
					}

					if(_irqCounter == 0 && _irqEnabled) {
						_console->GetCpu()->SetIrqSource(IRQSource::External);
					}
					_irqReload = false;
					break;
			}
		}
	}

	void UpdatePrg()
	{
		switch(_mode & 0x03) {
			case 0:
				SelectPrgPage(0, _vrc2Prg[0]);
				SelectPrgPage(1, _vrc2Prg[1]);
				SelectPrgPage(2, -2);
				SelectPrgPage(3, -1);
				break;

			case 1: {
				uint32_t prgMode = (_mmc3Ctrl >> 5) & 0x02;
				SelectPrgPage(0, _mmc3Regs[6 + prgMode]);
				SelectPrgPage(1, _mmc3Regs[7]);
				SelectPrgPage(2, _mmc3Regs[6 + (prgMode ^ 0x02)]);
				SelectPrgPage(3, _mmc3Regs[9]);
				break;
			}

			case 2:
			case 3: {
				uint8_t bank = _mmc1Regs[3] & 0x0F;
				if(_mmc1Regs[0] & 0x08) {
					if(_mmc1Regs[0] & 0x04) {
						SelectPrgPage2x(0, bank << 1);
						SelectPrgPage2x(1, 0x0F << 1);
					} else {
						SelectPrgPage2x(0, 0);
						SelectPrgPage2x(1, bank << 1);
					}
				} else {
					SelectPrgPage4x(0, (bank & 0xFE) << 1);
				}
				break;
			}
		}
	}

	void UpdateChr()
	{
		uint32_t outerBank = (_mode & 0x04) << 6;
		switch(_mode & 0x03) {
			case 0:
				for(int i = 0; i < 8; i++) {
					SelectChrPage(i, outerBank | _vrc2Chr[i]);
				}
				break;

			case 1: {
				uint32_t slotSwap = (_mmc3Ctrl & 0x80) ? 4 : 0;
				SelectChrPage(0 ^ slotSwap, outerBank | ((_mmc3Regs[0]) & 0xFE));
				SelectChrPage(1 ^ slotSwap, outerBank | (_mmc3Regs[0] | 1));
				SelectChrPage(2 ^ slotSwap, outerBank | ((_mmc3Regs[1]) & 0xFE));
				SelectChrPage(3 ^ slotSwap, outerBank | (_mmc3Regs[1] | 1));
				SelectChrPage(4 ^ slotSwap, outerBank | _mmc3Regs[2]);
				SelectChrPage(5 ^ slotSwap, outerBank | _mmc3Regs[3]);
				SelectChrPage(6 ^ slotSwap, outerBank | _mmc3Regs[4]);
				SelectChrPage(7 ^ slotSwap, outerBank | _mmc3Regs[5]);
				break;
			}

			case 2:
			case 3: {
				if(_mmc1Regs[0] & 0x10) {
					SelectChrPage4x(0, _mmc1Regs[1] << 2);
					SelectChrPage4x(1, _mmc1Regs[2] << 2);
				} else {
					SelectChrPage8x(0, (_mmc1Regs[1] & 0xFE) << 2);
				}
				break;
			}
		}
	}

	void UpdateMirroring()
	{
		switch(_mode & 0x03) {
			case 0: SetMirroringType((_vrc2Mirroring & 0x01) ? MirroringType::Horizontal : MirroringType::Vertical); break;
			case 1: SetMirroringType((_mmc3Mirroring & 0x01) ? MirroringType::Horizontal : MirroringType::Vertical); break;

			case 2:
			case 3:
				switch(_mmc1Regs[0] & 0x03) {
					case 0: SetMirroringType(MirroringType::ScreenAOnly); break;
					case 1: SetMirroringType(MirroringType::ScreenBOnly); break;
					case 2: SetMirroringType(MirroringType::Vertical); break;
					case 3: SetMirroringType(MirroringType::Horizontal); break;
				}
				break;
		}
	}

	void UpdateState()
	{
		UpdatePrg();
		UpdateChr();
		UpdateMirroring();
	}

	void WriteVrc2Register(uint16_t addr, uint8_t value)
	{
		if(addr >= 0xB000 && addr <= 0xE003) {
			int32_t regIndex = ((((addr & 0x02) | (addr >> 10)) >> 1) + 2) & 0x07;
			int32_t lowHighNibble = ((addr & 1) << 2);
			_vrc2Chr[regIndex] = (_vrc2Chr[regIndex] & (0xF0 >> lowHighNibble)) | ((value & 0x0F) << lowHighNibble);
			UpdateChr();
		} else {
			switch(addr & 0xF000) {
				case 0x8000: _vrc2Prg[0] = value; UpdatePrg(); break;
				case 0xA000: _vrc2Prg[1] = value; UpdatePrg(); break;
				case 0x9000: _vrc2Mirroring = value; UpdateMirroring(); break;
			}
		}
	}

	void WriteMmc3Register(uint16_t addr, uint8_t value)
	{
		switch(addr & 0xE001) {
			case 0x8000:
				_mmc3Ctrl = value;
				UpdateState();
				break;

			case 0x8001:
				_mmc3Regs[_mmc3Ctrl & 0x07] = value;
				UpdateState();
				break;

			case 0xA000:
				_mmc3Mirroring = value;
				UpdateState();
				break;

			case 0xC000: _irqReloadValue = value; break;
			case 0xC001: _irqReload = true; break;
			
			case 0xE000:
				_console->GetCpu()->ClearIrqSource(IRQSource::External);
				_irqEnabled = false;
				break;
			
			case 0xE001: _irqEnabled = true; break;
		}
	}

	void WriteMmc1Register(uint16_t addr, uint8_t value)
	{
		if(value & 0x80) {
			_mmc1Regs[0] |= 0xc;
			_mmc1Buffer = _mmc1Shift = 0;
			UpdateState();
		} else {
			uint8_t regIndex = (addr >> 13) - 4;
			_mmc1Buffer |= (value & 0x01) << (_mmc1Shift++);
			if(_mmc1Shift == 5) {
				_mmc1Regs[regIndex] = _mmc1Buffer;
				_mmc1Buffer = _mmc1Shift = 0;
				UpdateState();
			}
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if((addr & 0x4100) == 0x4100) {
				_mode = value;
				if(addr & 0x01) {
					_mmc1Regs[0] = 0xc;
					_mmc1Regs[3] = 0;
					_mmc1Buffer = 0;
					_mmc1Shift = 0;
				}
				UpdateState();
			}
		} else {
			switch(_mode & 0x03) {
				case 0: WriteVrc2Register(addr, value); break;
				case 1: WriteMmc3Register(addr, value); break;

				case 2:
				case 3: WriteMmc1Register(addr, value); break;
			}
		}
	}
};
