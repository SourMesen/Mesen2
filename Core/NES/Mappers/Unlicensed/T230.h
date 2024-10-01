#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/Mappers/Konami/VrcIrq.h"

class T230 : public BaseMapper
{
private:
	unique_ptr<VrcIrq> _irq;

	uint8_t _prgReg0 = 0;
	uint8_t _prgReg1 = 0;
	uint8_t _prgMode = 0;

	uint16_t _outerBank = 0;

	uint8_t _hiCHRRegs[8] = {};
	uint8_t _loCHRRegs[8] = {};

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x0400; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		_irq.reset(new VrcIrq(_console));

		_prgMode = GetPowerOnByte() & 0x01;
		_prgReg0 = GetPowerOnByte() & 0x1F;
		_prgReg1 = GetPowerOnByte() & 0x1F;

		for(int i = 0; i < 8; i++) {
			_loCHRRegs[i] = GetPowerOnByte() & 0x0F;
			_hiCHRRegs[i] = GetPowerOnByte() & 0x1F;
		}

		UpdateState();
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();
		_irq->ProcessCpuClock();
	}

	void UpdateState()
	{
		if(_chrRamSize) {
			SelectChrPage8x(0, 0);
		} else {
			for(int i = 0; i < 8; i++) {
				SelectChrPage(i, _loCHRRegs[i] | (_hiCHRRegs[i] << 4));
			}
		}

		if(_prgMode == 0) {
			SelectPrgPage(0, _prgReg0 | _outerBank);
			SelectPrgPage(2, (-2 & 0x1F) | _outerBank);
		} else {
			SelectPrgPage(0, (-2 & 0x1F) | _outerBank);
			SelectPrgPage(2, _prgReg0 | _outerBank);
		}
		SelectPrgPage(1, _prgReg1);
		SelectPrgPage(3, -1);

	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		addr = (addr & 0xF000) | (addr & 0x2A ? 0x02 : 0) | (addr & 0x15 ? 0x01 : 0);

		if(addr >= 0x9000 && addr <= 0x9001) {
			switch(value) {
				case 0: SetMirroringType(MirroringType::Vertical); break;
				case 1: SetMirroringType(MirroringType::Horizontal); break;
				case 2: SetMirroringType(MirroringType::ScreenAOnly); break;
				case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
			}
		} else if(addr >= 0x9002 && addr <= 0x9003) {
			_prgMode = (value >> 1) & 0x01;
		} else if(addr >= 0xA000 && addr <= 0xA003) {
			_prgReg0 = (value & 0x1F) << 1;
			_prgReg1 = ((value & 0x1F) << 1) | 0x01;
		} else if(addr >= 0xB000 && addr <= 0xE003) {
			if(_chrRamSize > 0) {
				_outerBank = (value & 0x08) << 2;
			} else {
				uint8_t regNumber = ((((addr >> 12) & 0x07) - 3) << 1) + ((addr >> 1) & 0x01);
				bool lowBits = (addr & 0x01) == 0x00;
				if(lowBits) {
					//The other reg contains the low 4 bits
					_loCHRRegs[regNumber] = value & 0x0F;
				} else {
					//One reg contains the high 5 bits 
					_hiCHRRegs[regNumber] = value & 0x1F;
				}
			}			
		} else if(addr == 0xF000) {
			_irq->SetReloadValueNibble(value, false);
		} else if(addr == 0xF001) {
			_irq->SetReloadValueNibble(value, true);
		} else if(addr == 0xF002) {
			_irq->SetControlValue(value);
		} else if(addr == 0xF003) {
			_irq->AcknowledgeIrq();
		}

		UpdateState();
	}

public:
	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_loCHRRegs, 8);
		SVArray(_hiCHRRegs, 8);
		SV(_irq);
		SV(_prgReg0);
		SV(_prgReg1);
		SV(_prgMode);
	}
};