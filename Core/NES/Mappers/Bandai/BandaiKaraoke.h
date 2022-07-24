#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "NES/NesMemoryManager.h"
#include "NES/NesControlManager.h"
#include "NES/Input/BandaiMicrophone.h"

class BandaiKaraoke : public BaseMapper
{
private:
	shared_ptr<BandaiMicrophone> _microphone;

protected:
	uint16_t GetPRGPageSize() override { return 0x4000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	bool AllowRegisterRead() override { return true; }
	bool HasBusConflicts() override { return true; }

	void InitMapper() override
	{
		AddRegisterRange(0x6000, 0x7FFF, MemoryOperation::Read);
		RemoveRegisterRange(0x8000, 0xFFFF, MemoryOperation::Read);

		SelectPRGPage(0, 0);
		SelectPRGPage(1, 0x07);
		SelectCHRPage(0, 0);

		//TODO register in control manager
		_microphone.reset(new BandaiMicrophone(_emu, _emu->GetSettings()->GetNesConfig().Port1.Keys));
	}

	uint8_t ReadRegister(uint16_t addr) override
	{
		return _microphone->ReadRam(addr) | _console->GetMemoryManager()->GetOpenBus(0xF8);
	}

	void WriteRegister(uint16_t addr, uint8_t value) override
	{
		if(value & 0x10) {
			//Select internal rom
			SelectPRGPage(0, value & 0x07);
		} else {
			//Select expansion rom
			if(_prgSize >= 0x40000) {
				SelectPRGPage(0, (value & 0x07) | 0x08);
			} else {
				//Open bus for roms that don't contain the expansion rom
				RemoveCpuMemoryMapping(0x8000, 0xBFFF);
			}
		}

		SetMirroringType(value & 0x20 ? MirroringType::Horizontal : MirroringType::Vertical);
	}
};