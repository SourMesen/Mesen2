#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesCpu.h"
#include "NES/NesConsole.h"

class Sunsoft4 : public BaseMapper
{
private:
	uint8_t _ntRegs[2] = {};
	bool _useChrForNametables = false;
	bool _prgRamEnabled = false;
	uint32_t _licensingTimer = 0;
	bool _usingExternalRom = false;
	uint8_t _externalPage = 0;

	void UpdateNametables()
	{
		if(_useChrForNametables) {
			for(int i = 0; i < 4; i++) {
				uint8_t reg = 0;
				switch(GetMirroringType()) {
					case MirroringType::FourScreens: break; //4-screen mirroring is not supported by this mapper
					case MirroringType::Vertical: reg = i & 0x01; break;
					case MirroringType::Horizontal: reg = (i & 0x02) >> 1; break;
					case MirroringType::ScreenAOnly: reg = 0; break;
					case MirroringType::ScreenBOnly: reg = 1; break;
				}

				SetPpuMemoryMapping(0x2000+i*0x400, 0x2000+i*0x400+0x3FF, ChrMemoryType::Default, _ntRegs[reg] * 0x400, _chrRamSize > 0 ? MemoryAccessType::ReadWrite : MemoryAccessType::Read);
			}
		} else {
			//Reset to default mirroring
			SetMirroringType(GetMirroringType());
		}
	}

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x800; }
	bool EnableCpuClockHook() override { return true; }

	void InitMapper() override
	{
		_useChrForNametables = false;
		_ntRegs[0] = _ntRegs[1] = 0;
		
		_licensingTimer = 0;
		_usingExternalRom = false;
		_prgRamEnabled = false;

		//Bank 0's initial state is undefined, but some roms expect it to be the first page
		SelectPrgPage(0, 0);
		SelectPrgPage(1, 7);

		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		
		SV(_ntRegs[0]);
		SV(_ntRegs[1]);
		SV(_useChrForNametables);
		SV(_prgRamEnabled);
		SV(_usingExternalRom);
		SV(_externalPage);
	}

	void UpdateState()
	{
		MemoryAccessType access = _prgRamEnabled ? MemoryAccessType::ReadWrite : MemoryAccessType::NoAccess;
		SetCpuMemoryMapping(0x6000, 0x7FFF, 0, HasBattery() ? PrgMemoryType::SaveRam : PrgMemoryType::WorkRam, access);
		
		if(_usingExternalRom) { 
			if(_licensingTimer == 0) {
				RemoveCpuMemoryMapping(0x8000, 0xBFFF);
			} else {
				SelectPrgPage(0, _externalPage);
			}
		}
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_licensingTimer) {
			_licensingTimer--;
			if(_licensingTimer == 0) {
				UpdateState();
			}
		}
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		if(addr >= 0x6000 && addr <= 0x7FFF) {
			_licensingTimer = 1024 * 105;
			UpdateState();
		}
		BaseMapper::WriteRam(addr, value);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr & 0xF000) {
			case 0x8000: SelectChrPage(0, value); break;
			case 0x9000: SelectChrPage(1, value); break;
			case 0xA000: SelectChrPage(2, value); break;
			case 0xB000: SelectChrPage(3, value); break;
			case 0xC000: 
				_ntRegs[0] = value | 0x80;
				UpdateNametables();
				break;
			case 0xD000:
				_ntRegs[1] = value | 0x80;
				UpdateNametables();
				break;
			case 0xE000:
				switch(value & 0x03) {
					case 0: SetMirroringType(MirroringType::Vertical); break;
					case 1: SetMirroringType(MirroringType::Horizontal); break;
					case 2: SetMirroringType(MirroringType::ScreenAOnly); break;
					case 3: SetMirroringType(MirroringType::ScreenBOnly); break;
				}
				_useChrForNametables = (value & 0x10) == 0x10;
				UpdateNametables();
				break;
			case 0xF000: 
				bool externalPrg = (value & 0x08) == 0;
				if(externalPrg && GetPrgPageCount() > 8) {
					_usingExternalRom = true;
					_externalPage = 0x08 | ((value & 0x07) % (GetPrgPageCount() - 0x08));
					SelectPrgPage(0, _externalPage);
				} else {
					_usingExternalRom = false;
					SelectPrgPage(0, value & 0x07);
				}

				_prgRamEnabled = (value & 0x10) == 0x10;
				UpdateState();

				break;
		}
	}
};