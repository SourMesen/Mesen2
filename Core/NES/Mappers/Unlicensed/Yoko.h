#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesCpu.h"
#include "NES/NesConsole.h"
#include "NES/NesMemoryManager.h"

class Yoko : public BaseMapper
{
	uint8_t _regs[7] = {};
	uint8_t _exRegs[4] = {};
	uint8_t _mode = 0;
	uint8_t _bank = 0;
	uint16_t _irqCounter = 0;
	bool _irqEnabled = false;

protected:
	uint32_t GetDipSwitchCount() override { return 2; }
	uint16_t RegisterStartAddress() override { return 0x5000; }
	uint16_t RegisterEndAddress() override { return 0x5FFF; }
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x800; }
	bool AllowRegisterRead() override { return true; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		memset(_regs, 0, sizeof(_regs));
		memset(_exRegs, 0, sizeof(_exRegs));
		_mode = 0;
		_bank = 0;
		_irqCounter = 0;
		_irqEnabled = false;

		RemoveRegisterRange(0x5000, 0x53FF, MemoryOperation::Write);
		AddRegisterRange(0x8000, 0xFFFF, MemoryOperation::Write);

		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);

		SVArray(_regs, 7);
		SVArray(_exRegs, 4);
		SV(_mode);
		SV(_bank);
		SV(_irqCounter);
		SV(_irqEnabled);
	}

	void Reset(bool softReset) override
	{
		if(softReset) {
			_mode = 0;
			_bank = 0;
		}
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_irqEnabled) {
			_irqCounter--;
			if(_irqCounter == 0) {
				_irqEnabled = false;
				_irqCounter = 0xFFFF;
				_console->GetCpu()->SetIrqSource(IRQSource::External);
			}
		}
	}

	void UpdateState()
	{
		SetMirroringType(_mode & 0x01 ? MirroringType::Horizontal : MirroringType::Vertical);

		SelectChrPage(0, _regs[3]);
		SelectChrPage(1, _regs[4]);
		SelectChrPage(2, _regs[5]);
		SelectChrPage(3, _regs[6]);

		if(_mode & 0x10) {
			uint32_t outer = (_bank & 0x08) << 1;
			SelectPrgPage(0, outer | (_regs[0] & 0x0F));
			SelectPrgPage(1, outer | (_regs[1] & 0x0F));
			SelectPrgPage(2, outer | (_regs[2] & 0x0F));
			SelectPrgPage(3, outer | 0x0F);
		} else if(_mode & 0x08) {
			SelectPrgPage4x(0, (_bank & 0xFE) << 1);
		} else {
			SelectPrgPage2x(0, _bank << 1);
			SelectPrgPage2x(1, -2);
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if(addr <= 0x53FF) {
			return (_console->GetMemoryManager()->GetOpenBus() & 0xFC) | GetDipSwitches();
		} else {
			return _exRegs[addr & 0x03];
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			_exRegs[addr & 0x03] = value;
		} else {
			switch(addr & 0x8C17) {
				case 0x8000: _bank = value; UpdateState(); break;
				case 0x8400: _mode = value; UpdateState(); break;
				case 0x8800:
					_irqCounter = (_irqCounter & 0xFF00) | value;
					_console->GetCpu()->ClearIrqSource(IRQSource::External);
					break;
				case 0x8801:
					_irqEnabled = (_mode & 0x80) != 0;
					_irqCounter = (_irqCounter & 0xFF) | (value << 8);
					break;
				case 0x8c00: _regs[0] = value; UpdateState(); break;
				case 0x8c01: _regs[1] = value; UpdateState(); break;
				case 0x8c02: _regs[2] = value; UpdateState(); break;
				case 0x8c10: _regs[3] = value; UpdateState(); break;
				case 0x8c11: _regs[4] = value; UpdateState(); break;
				case 0x8c16: _regs[5] = value; UpdateState(); break;
				case 0x8c17: _regs[6] = value; UpdateState(); break;
			}
		}
	}
};
