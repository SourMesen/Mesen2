#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC1.h"
#include "NES/NesConsole.h"
#include "NES/NesMemoryManager.h"
#include "Utilities/Serializer.h"
#include "Utilities/FolderUtilities.h"

class FnsMmc1 : public MMC1
{
private:
	vector<uint8_t> _kanjiRomData;
	MirroringType _mirroringSelect = MirroringType::Vertical;
	uint8_t _kanjiRomPos = 0;
	uint8_t _kanjiRomBank = 0;
	uint8_t _chrRamBank = 0;
	bool _workRamEnable1 = true;
	bool _workRamEnable2 = false;

public:
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		MMC1::InitMapper();

		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);
		AddRegisterRange(0x40AE, 0x40C0, MemoryOperation::Any);
		AddRegisterRange(0x5000, 0x5FFF, MemoryOperation::Read);

		VirtualFile chrData(FolderUtilities::CombinePath(FolderUtilities::GetFirmwareFolder(), "lh5323m1.bin"));
		if(chrData.IsValid()) {
			chrData.ReadFile(_kanjiRomData);
		} else {
			_kanjiRomData.resize(256 * 1024);
		}
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x6000) {
			switch(addr) {
				case 0x40AE: _workRamEnable1 = value & 0x01; UpdateState();  break;
				case 0x40B0: _kanjiRomBank = value & 0x01; break;
				case 0x40C0: 
					_workRamEnable2 = value & 0x01;
					_chrRamBank = (value & 0x08) >> 3;
					UpdateState();
					break;
				case 0x40AD: _mirroringSelect = value & 0x80 ? MirroringType::Horizontal : MirroringType::Vertical; UpdateState(); break;
			}
		} else {
			MMC1::WriteRegister(addr, value);
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		if(addr < 0x5000) {
			switch(addr) {
				case 0x40B0: _kanjiRomPos = 0; break;
				case 0x40C0: return 0x80;
				default: break;
			}
			return _console->GetMemoryManager()->GetOpenBus();
		} else if(addr < 0x6000) {
			uint8_t value = _kanjiRomData[(_kanjiRomBank ? 0x20000 : 0) | ((addr & 0xFFF) << 5) | _kanjiRomPos];
			_kanjiRomPos = (_kanjiRomPos + 1) & 0x1F;
			return value;
		} else {
			return MMC1::ReadRegister(addr);
		}
	}

	void UpdateState() override
	{
		MMC1::UpdateState();
		SetMirroringType(_mirroringSelect);
		SetPpuMemoryMapping(0x0000, 0x1FFF, ChrMemoryType::ChrRam, _chrRamBank ? 0x2000 : 0, MemoryAccessType::ReadWrite);
		if(_workRamEnable1 && _workRamEnable2) {
			SetCpuMemoryMapping(0x6000, 0x7FFF, PrgMemoryType::WorkRam, 0, MemoryAccessType::ReadWrite);
		}
	}

	void Serialize(Serializer& s) override
	{
		MMC1::Serialize(s);
		SV(_mirroringSelect);
		SV(_kanjiRomPos);
		SV(_kanjiRomBank);
		SV(_chrRamBank);
		SV(_workRamEnable1);
		SV(_workRamEnable2);
	}
};