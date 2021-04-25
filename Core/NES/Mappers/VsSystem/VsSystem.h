#pragma once
#include "stdafx.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "Shared/Interfaces/IControlManager.h"
#include "NES/Mappers/VsSystem/VsControlManager.h"
#include "Utilities/Serializer.h"

class VsSystem : public BaseMapper
{
private:
	uint8_t _prgChrSelectBit = 0;

protected:
	uint16_t GetPRGPageSize() override { return 0x2000; }
	uint16_t GetCHRPageSize() override { return 0x2000; }
	uint32_t GetWorkRamSize() override { return 0x800; }

	void InitMapper() override
	{
		if(!IsNes20()) {
			//Force VS system if mapper 99
			_romInfo.System = GameSystem::VsSystem;
			if(_prgSize >= 0x10000) {
				_romInfo.VsType = VsSystemType::VsDualSystem;
			} else {
				_romInfo.VsType = VsSystemType::Default;
			}
		}

		//"Note: unlike all other mappers, an undersize mapper 99 image implies open bus instead of mirroring."
		//However, it doesn't look like any game actually rely on this behavior?  So not implemented for now.
		bool initialized = false;
		if(_prgSize == 0xC000) {
			//48KB rom == unpadded dualsystem rom
			if(_romInfo.VsType == VsSystemType::VsDualSystem) {
				uint8_t prgOuter = _console->IsVsMainConsole() ? 0 : 3;
				SelectPRGPage(1, 0 + prgOuter);
				SelectPRGPage(2, 1 + prgOuter);
				SelectPRGPage(3, 2 + prgOuter);
				initialized = true;
			} else if(_romInfo.VsType == VsSystemType::RaidOnBungelingBayProtection) {
				if(_console->IsVsMainConsole()) {
					SelectPRGPage(0, 0);
					SelectPRGPage(1, 1);
					SelectPRGPage(2, 2);
					SelectPRGPage(3, 3);
				} else {
					//Slave CPU
					SelectPRGPage(0, 4);
				}
				initialized = true;
			}
		}

		if(!initialized) {
			uint8_t prgOuter = _console->IsVsMainConsole() ? 0 : 4;
			SelectPRGPage(0, 0 | prgOuter);
			SelectPRGPage(1, 1 | prgOuter);
			SelectPRGPage(2, 2 | prgOuter);
			SelectPRGPage(3, 3 | prgOuter);
		}

		uint8_t chrOuter = _console->IsVsMainConsole() ? 0 : 2;
		SelectCHRPage(0, 0 | chrOuter);
	}

	void Reset(bool softReset) override
	{
		BaseMapper::Reset(softReset);
		UpdateMemoryAccess(0);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		s.Stream(_prgChrSelectBit);
	}

	void ProcessCpuClock() override
	{
		VsControlManager* controlManager = dynamic_cast<VsControlManager*>(_console->GetControlManager().get());
		if(controlManager && _prgChrSelectBit != controlManager->GetPrgChrSelectBit()) {
			_prgChrSelectBit = controlManager->GetPrgChrSelectBit();

			if(_romInfo.VsType == VsSystemType::Default && _prgSize > 0x8000) {
				//"Note: In case of games with 40KiB PRG - ROM(as found in VS Gumshoe), the above bit additionally changes 8KiB PRG - ROM at $8000 - $9FFF."
				//"Only Vs. Gumshoe uses the 40KiB PRG variant; in the iNES encapsulation, the 8KiB banks are arranged as 0, 1, 2, 3, 0alternate, empty"
				SelectPRGPage(0, _prgChrSelectBit << 2);
			}

			uint8_t chrOuter = _console->IsVsMainConsole() ? 0 : 2;
			SelectCHRPage(0, _prgChrSelectBit | chrOuter);
		}
	}

public:
	void UpdateMemoryAccess(uint8_t slaveMasterBit)
	{
		NesConsole* subConsole = _console->GetVsSubConsole();
		if(subConsole) {
			VsSystem* otherMapper = dynamic_cast<VsSystem*>(subConsole->GetMapper());

			//Give memory access to master CPU or slave CPU, based on "slaveMasterBit"
			if(_saveRamSize == 0 && _workRamSize == 0) {
				RemoveCpuMemoryMapping(0x6000, 0x7FFF);
				otherMapper->RemoveCpuMemoryMapping(0x6000, 0x7FFF);
			}

			uint8_t* memory = HasBattery() ? _saveRam : _workRam;
			for(int i = 0; i < 4; i++) {
				SetCpuMemoryMapping(0x6000 + i * 0x800, 0x67FF + i * 0x800, memory, slaveMasterBit ? MemoryAccessType::ReadWrite : MemoryAccessType::NoAccess);
				otherMapper->SetCpuMemoryMapping(0x6000 + i * 0x800, 0x67FF + i * 0x800, memory, slaveMasterBit ? MemoryAccessType::NoAccess : MemoryAccessType::ReadWrite);
			}
		}
	}
};
