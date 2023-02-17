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
	bool HasBusConflicts() override { return !HasBattery(); }
	bool AllowRegisterRead() override { return HasBattery(); }

	void InitMapper() override
	{
		_flash.reset(new FlashSST39SF040(_prgRom, _prgSize));
		SelectPrgPage(0, 0);
		SelectPrgPage(1, -1);

		_enableMirroringBit = false;
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
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);

		SV(_flash);
		SV(_prgBank);

		if(s.IsSaving()) {
			vector<uint8_t> prgRom = vector<uint8_t>(_prgRom, _prgRom + _prgSize);
			vector<uint8_t> ipsData = IpsPatcher::CreatePatch(_orgPrgRom, prgRom);
			SVVector(ipsData);
		} else {
			vector<uint8_t> ipsData;
			SVVector(ipsData);

			vector<uint8_t> patchedPrgRom;
			if(IpsPatcher::PatchBuffer(ipsData, _orgPrgRom, patchedPrgRom)) {
				memcpy(_prgRom, patchedPrgRom.data(), _prgSize);
			}
		}
	}

	void ApplySaveData()
	{
		if(_console->GetNesConfig().DisableFlashSaves) {
			return;
		}

		//Apply save data (saved as an IPS file), if found
		vector<uint8_t> ipsData = _emu->GetBatteryManager()->LoadBattery(".ips");
		if(!ipsData.empty()) {
			vector<uint8_t> patchedPrgRom;
			if(IpsPatcher::PatchBuffer(ipsData, _orgPrgRom, patchedPrgRom)) {
				memcpy(_prgRom, patchedPrgRom.data(), _prgSize);
			}
		}
	}

	void SaveBattery() override
	{
		if(_console->GetNesConfig().DisableFlashSaves) {
			return;
		}

		if(HasBattery()) {
			vector<uint8_t> prgRom = vector<uint8_t>(_prgRom, _prgRom + _prgSize);
			vector<uint8_t> ipsData = IpsPatcher::CreatePatch(_orgPrgRom, prgRom);
			if(ipsData.size() > 8) {
				_emu->GetBatteryManager()->SaveBattery(".ips", ipsData.data(), (uint32_t)ipsData.size());
			}
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
				SetMirroringType(value & 0x80 ? MirroringType::ScreenBOnly : MirroringType::ScreenAOnly);
			}
		} else {
			_flash->Write((addr & 0x3FFF) | (_prgBank << 14), value);
		}
	}
};
