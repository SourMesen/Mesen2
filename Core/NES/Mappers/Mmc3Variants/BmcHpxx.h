#pragma once
#include "pch.h"
#include "NES/Mappers/Nintendo/MMC3.h"

class BmcHpxx : public MMC3
{
private:
	uint8_t _exRegs[5] = {};
	bool _locked = false;

protected:
	uint32_t GetDipSwitchCount() override { return 4; }
	bool AllowRegisterRead() override { return true; }

	void InitMapper() override
	{
		memset(_exRegs, 0, sizeof(_exRegs));
		_locked = false;

		MMC3::InitMapper();
		AddRegisterRange(0x5000, 0x5FFF, MemoryOperation::Any);
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);
	}

	void Reset(bool softReset) override
	{
		MMC3::Reset(softReset);
		memset(_exRegs, 0, sizeof(_exRegs));
		_locked = false;
		MMC3::ResetMmc3();
		UpdateState();
	}

	void Serialize(Serializer& s) override
	{
		MMC3::Serialize(s);
		SVArray(_exRegs, 5);
		SV(_locked);
	}

	void SelectChrPage(uint16_t slot, uint16_t page, ChrMemoryType memoryType = ChrMemoryType::Default) override
	{
		if(_exRegs[0] & 0x04) {
			switch(_exRegs[0] & 0x03) {
				case 0:
				case 1: SelectChrPage8x(0, (_exRegs[2] & 0x3F) << 3); break;
				case 2: SelectChrPage8x(0, ((_exRegs[2] & 0x3E) | (_exRegs[4] & 0x01)) << 3); break;
				case 3: SelectChrPage8x(0, ((_exRegs[2] & 0x3C) | (_exRegs[4] & 0x03)) << 3); break;
			}
		} else {
			uint8_t base, mask;
			if(_exRegs[0] & 0x01) {
				base = _exRegs[2] & 0x30;
				mask = 0x7F;
			} else {
				base = _exRegs[2] & 0x20;
				mask = 0xFF;
			}
			MMC3::SelectChrPage(slot, (page & mask) | (base << 3));
		}
	}

	void SelectPrgPage(uint16_t slot, uint16_t page, PrgMemoryType memoryType = PrgMemoryType::PrgRom) override
	{
		if(_exRegs[0] & 0x04) {
			if((_exRegs[0] & 0x0F) == 0x04) {
				SelectPrgPage2x(0, (_exRegs[1] & 0x1F) << 1);
				SelectPrgPage2x(1, (_exRegs[1] & 0x1F) << 1);
			} else {
				SelectPrgPage4x(0, (_exRegs[1] & 0x1E) << 1);
			}
		} else {
			uint8_t base, mask;
			if(_exRegs[0] & 0x02) {
				base = _exRegs[1] & 0x18;
				mask = 0x0F;
			} else {
				base = _exRegs[1] & 0x10;
				mask = 0x1F;
			}
			MMC3::SelectPrgPage(slot, (page & mask) | (base << 1));
		}
	}

	void UpdateMirroring() override
	{
		if(_exRegs[0] & 0x04) {
			SetMirroringType(_exRegs[4] & 0x04 ? MirroringType::Vertical : MirroringType::Horizontal);
		} else {
			MMC3::UpdateMirroring();
		}
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		return GetDipSwitches();
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(addr < 0x8000) {
			if(!_locked) {
				_exRegs[addr & 0x03] = value;
				_locked = (value & 0x80) != 0;
				UpdatePrgMapping();
				UpdateChrMapping();
			}
		} else {
			if(_exRegs[0] & 0x04) {
				_exRegs[4] = value;
				UpdateChrMapping();
			} else {
				MMC3::WriteRegister(addr, value);
			}
		}
	}
};