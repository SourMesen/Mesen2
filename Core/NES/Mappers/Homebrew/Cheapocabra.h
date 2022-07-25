#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"
#include "NES/Mappers/Homebrew/FlashSST39SF040.h"
#include "Shared/Emulator.h"
#include "Shared/BatteryManager.h"
#include "Utilities/Patches/IpsPatcher.h"

class Cheapocabra : public BaseMapper
{
private:
	unique_ptr<FlashSST39SF040> _flash;
	uint8_t _prgReg = 0;
	vector<uint8_t> _orgPrgRom;

protected:
	uint16_t GetPrgPageSize() override { return 0x8000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint32_t GetWorkRamSize() override { return 0; }
	uint32_t GetSaveRamSize() override { return 0; }
	uint16_t RegisterStartAddress() override { return 0x5000; }
	uint16_t RegisterEndAddress() override { return 0x5FFF; }
	uint32_t GetChrRamSize() override { return 0x4000; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		AddRegisterRange(0x7000, 0x7FFF, MemoryOperation::Write);

		_flash.reset(new FlashSST39SF040(_prgRom, _prgSize));
		AddRegisterRange(0x8000, 0xFFFF, MemoryOperation::Any);
		RemoveRegisterRange(0x5000, 0x5FFF, MemoryOperation::Read);
		
		WriteRegister(0x5000, GetPowerOnByte());

		_orgPrgRom = vector<uint8_t>(_prgRom, _prgRom + _prgSize);
		ApplySaveData();
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		
		SV(_flash);
		SV(_prgReg);
	}

	void ApplySaveData()
	{
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
		vector<uint8_t> prgRom = vector<uint8_t>(_prgRom, _prgRom + _prgSize);
		vector<uint8_t> ipsData = IpsPatcher::CreatePatch(_orgPrgRom, prgRom);
		if(ipsData.size() > 8) {
			_emu->GetBatteryManager()->SaveBattery(".ips", ipsData.data(), (uint32_t)ipsData.size());
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
		if(addr < 0x8000) {
			_prgReg = value & 0x0F;
			SelectPrgPage(0, _prgReg);

			SelectChrPage(0, (value >> 4) & 0x01);
			for(int i = 0; i < 8; i++) {
				SetNametable(i, ((value & 0x20) ? 8 : 0) + i);
			}
		} else {
			_flash->Write((_prgReg << 15) | (addr & 0x7FFF), value);
		}
	}
};