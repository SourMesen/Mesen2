#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/Mappers/Homebrew/FlashSST39SF040.h"
#include "Shared/BatteryManager.h"
#include "Utilities/Patches/IpsPatcher.h"
#include "Utilities/Serializer.h"

class UnRom512 : public BaseMapper
{
private:
	unique_ptr<FlashSST39SF040> _flash;
	bool _enableMirroringBit = false;
	uint8_t _prgBank = 0;
	vector<uint8_t> _orgPrgRom;

protected:
	uint16_t GetPrgPageSize() override { return 0x4000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint32_t GetWorkRamSize() override { return 0; }
	uint32_t GetSaveRamSize() override { return 0; }
	uint16_t RegisterStartAddress() override { return 0x8000; }
	uint16_t RegisterEndAddress() override { return 0xFFFF; }
	uint32_t GetChrRamSize() override { return 0x8000; }
	bool HasBusConflicts() override { return (_romInfo.SubMapperID == 0 && !HasBattery()) || _romInfo.SubMapperID == 2; }
	bool AllowRegisterRead() override { return HasBattery(); }

	void InitMapper() override
	{
		_flash.reset(new FlashSST39SF040(_prgRom, _prgSize));
		SelectPrgPage(0, 0);
		SelectPrgPage(1, -1);

		_enableMirroringBit = false;
		if(_romInfo.SubMapperID == 3) {
			_enableMirroringBit = true;
			SetMirroringType(MirroringType::Vertical);
		} else {
			if(GetMirroringType() == MirroringType::ScreenAOnly || GetMirroringType() == MirroringType::ScreenBOnly) {
				SetMirroringType(MirroringType::ScreenAOnly);
				_enableMirroringBit = true;
			} else {
				switch(_romInfo.Header.Byte6 & 0x09) {
					case 0: SetMirroringType(MirroringType::Horizontal); break;
					case 1: SetMirroringType(MirroringType::Vertical); break;
					case 8: SetMirroringType(MirroringType::ScreenAOnly); _enableMirroringBit = true; break;
					case 9: SetMirroringType(MirroringType::FourScreens); break;
				}
			}
		}

		if(GetMirroringType() == MirroringType::FourScreens && _chrRam && _chrRamSize >= 0x8000) {
			//InfiniteNesLives four-screen mirroring variation, last 8kb of CHR RAM is always mapped to 0x2000-0x3FFF (0x3EFF due to palette)
			//This "breaks" the "UNROM512_4screen_test" test ROM - was the ROM actually tested on this board? Seems to contradict hardware specs
			SetPpuMemoryMapping(0x2000, 0x3FFF, ChrMemoryType::ChrRam, 0x6000, MemoryAccessType::ReadWrite);
		}

		if(HasBattery()) {
			AddRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);
			_orgPrgRom = vector<uint8_t>(_prgRom, _prgRom + _prgSize);
			ApplySaveData();
		}

		if(_romInfo.SubMapperID == 4) {
			//LED register is at 0x8000-0xBFFF and currently not emulated
			RemoveRegisterRange(0x8000, 0xBFFF, MemoryOperation::Write);
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);

		SV(_flash);
		SV(_prgBank);
		
		SerializeRomDiff(s, _orgPrgRom);
	}

	void ApplySaveData()
	{
		if(_console->GetNesConfig().DisableFlashSaves) {
			return;
		}

		LoadRomPatch(_orgPrgRom);
	}

	void SaveBattery() override
	{
		if(_console->GetNesConfig().DisableFlashSaves) {
			return;
		}

		if(HasBattery()) {
			SaveRom(_orgPrgRom);
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		int16_t value = _flash->Read(addr);
		if(value >= 0) {
			return (uint8_t)value;
		}

		return BaseMapper::InternalReadRam(addr);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(!HasBattery() || addr >= 0xC000) {
			SelectPrgPage(0, value & 0x1F);
			_prgBank = value & 0x1F;

			SelectChrPage(0, (value >> 5) & 0x03);

			if(_enableMirroringBit) {
				if(_romInfo.SubMapperID == 3) {
					SetMirroringType(value & 0x80 ? MirroringType::Vertical : MirroringType::Horizontal);
				} else {
					SetMirroringType(value & 0x80 ? MirroringType::ScreenBOnly : MirroringType::ScreenAOnly);
				}
			}
		} else {
			_flash->Write((addr & 0x3FFF) | (_prgBank << 14), value);
		}
	}
};
