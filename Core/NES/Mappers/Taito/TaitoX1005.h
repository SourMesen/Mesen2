#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"

class TaitoX1005 : public BaseMapper
{
private:
	bool _alternateMirroring = false;
	uint8_t _ramPermission = 0;

	void UpdateRamAccess()
	{
		SetCpuMemoryMapping(0x7F00, 0x7FFF, 0, HasBattery() ? PrgMemoryType::SaveRam : PrgMemoryType::WorkRam, _ramPermission == 0xA3 ? MemoryAccessType::ReadWrite : MemoryAccessType::NoAccess);
	}

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x0400; }
	uint16_t RegisterStartAddress() override { return 0x7EF0; }
	uint16_t RegisterEndAddress() override { return 0x7EFF; }

	uint32_t GetWorkRamSize() override { return 0x100; }
	uint32_t GetWorkRamPageSize() override { return 0x100; }
	uint32_t GetSaveRamSize() override { return 0x100; }
	uint32_t GetSaveRamPageSize() override { return 0x100; }

	bool ForceSaveRamSize() override { return HasBattery(); }
	bool ForceWorkRamSize() override { return !HasBattery(); }

	void InitMapper() override
	{
		_ramPermission = 0;

		SelectPrgPage(3, -1);

		UpdateRamAccess();
	}

	void WriteRam(uint16_t addr, uint8_t value) override
	{
		if((addr & 0xFF00) == 0x7F00) {
			//Mirror writes to the top/bottom - the mapper only has 128 bytes of ram, mirrored once.
			//The current BaseMapper code doesn't support mapping blocks smaller than 256 bytes,
			//so doing this at least ensures it behaves like a mirrored 128-byte block of ram
			BaseMapper::WriteRam(addr ^ 0x80, value);
		}
		BaseMapper::WriteRam(addr, value);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		switch(addr) {
			case 0x7EF0:
				SelectChrPage(0, value);
				SelectChrPage(1, value  + 1);
				if(_alternateMirroring) {
					SetNametable(0, value >> 7);
					SetNametable(1, value >> 7);
				}
				break;
			case 0x7EF1:
				SelectChrPage(2, value );
				SelectChrPage(3, value + 1);
				if(_alternateMirroring) {
					SetNametable(2, value >> 7);
					SetNametable(3, value >> 7);
				}
				break;

			case 0x7EF2: SelectChrPage(4, value); break;
			case 0x7EF3: SelectChrPage(5, value); break;
			case 0x7EF4: SelectChrPage(6, value); break;
			case 0x7EF5: SelectChrPage(7, value); break;

			case 0x7EF6: case 0x7EF7:
				if(!_alternateMirroring) {
					SetMirroringType((value & 0x01) == 0x01 ? MirroringType::Vertical : MirroringType::Horizontal);
				}
				break;

			case 0x7EF8: case 0x7EF9:
				_ramPermission = value; 
				UpdateRamAccess();
				break;

			case 0x7EFA: case 0x7EFB:
				SelectPrgPage(0, value);
				break;

			case 0x7EFC: case 0x7EFD:
				SelectPrgPage(1, value);
				break;

			case 0x7EFE: case 0x7EFF:
				SelectPrgPage(2, value);
				break;
		}
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_ramPermission);
		
		if(!s.IsSaving()) {
			UpdateRamAccess();
		}
	}

public:
	TaitoX1005(bool alternateMirroring) : _alternateMirroring(alternateMirroring)
	{
	}
};
