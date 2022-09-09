#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "NES/NesMemoryManager.h"

class Sachen74LS374N : public BaseMapper
{
private:
	uint8_t _currentRegister = 0;
	uint8_t _regs[8] = {};

protected:
	uint32_t GetDipSwitchCount() override { return _romInfo.MapperID == 150 ? 1 : 0; }
	uint16_t RegisterStartAddress() override { return 0x4100; }
	uint16_t RegisterEndAddress() override { return 0x7FFF; }
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		_currentRegister = 0;
		memset(_regs, 0, sizeof(_regs));
		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SVArray(_regs, 8);
		SV(_currentRegister);
	}

	void UpdateState()
	{
		uint8_t chrPage;
		if(_romInfo.MapperID == 150) {
			chrPage = ((_regs[4] & 0x01) << 2) | (_regs[6] & 0x03);
		} else {
			chrPage = (_regs[2] & 0x01) | ((_regs[4] & 0x01) << 1) | ((_regs[6] & 0x03) << 2);
		}
		SelectChrPage(0, chrPage);
		SelectPrgPage(0, _regs[5] & 0x03);

		switch((_regs[7] >> 1) & 0x03) {
			case 0: SetNametables(0, 0, 0, 1); break;
			case 1: SetMirroringType(MirroringType::Horizontal); break;
			case 2: SetMirroringType(MirroringType::Vertical); break;
			case 3: SetMirroringType(MirroringType::ScreenAOnly); break;
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		uint8_t openBus = _console->GetMemoryManager()->GetOpenBus();
		if((addr & 0xC101) == 0x4101) {
			if(GetDipSwitches() & 0x01) {
				//"In the latter setting, the ASIC sees all writes as being OR'd with $04, while on reads, D2 is open bus."
				return (openBus & 0xFC) | (_regs[_currentRegister] & 0x03);
			} else {
				return (openBus & 0xF8) | (_regs[_currentRegister] & 0x07);
			}
		}
		return openBus;
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(GetDipSwitches() & 0x01) {
			//"In the latter setting, the ASIC sees all writes as being OR'd with $04, while on reads, D2 is open bus."
			value |= 0x04;
		}

		switch(addr & 0xC101) {
			case 0x4100: _currentRegister = value & 0x07; break;
			case 0x4101: _regs[_currentRegister] = (value & 0x07); UpdateState(); break;
		}
	}
};
