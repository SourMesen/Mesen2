#pragma once
#include "pch.h"
#include "NES/BaseMapper.h"
#include "NES/NesConsole.h"
#include "Shared/BaseControlManager.h"
#include "NES/Mappers/VsSystem/VsControlManager.h"
#include "Utilities/Serializer.h"

class VsSystem : public BaseMapper
{
private:
	uint8_t _prgChrSelectBit = 0;
	VsControlManager* _controlManager = nullptr;

protected:
	uint16_t GetPrgPageSize() override { return 0x2000; }
	uint16_t GetChrPageSize() override { return 0x2000; }
	uint32_t GetWorkRamSize() override { return 0x800; }
	bool EnableCpuClockHook() override { return true; }

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
				SelectPrgPage(1, 0 + prgOuter);
				SelectPrgPage(2, 1 + prgOuter);
				SelectPrgPage(3, 2 + prgOuter);
				initialized = true;
			} else if(_romInfo.VsType == VsSystemType::RaidOnBungelingBayProtection) {
				if(_console->IsVsMainConsole()) {
					SelectPrgPage(0, 0);
					SelectPrgPage(1, 1);
					SelectPrgPage(2, 2);
					SelectPrgPage(3, 3);
				} else {
					SelectPrgPage(0, 4);
				}
				initialized = true;
			}
		}

		if(!initialized) {
			uint8_t prgOuter = _console->IsVsMainConsole() ? 0 : 4;
			SelectPrgPage(0, 0 | prgOuter);
			SelectPrgPage(1, 1 | prgOuter);
			SelectPrgPage(2, 2 | prgOuter);
			SelectPrgPage(3, 3 | prgOuter);
		}

		uint8_t chrOuter = _console->IsVsMainConsole() ? 0 : 2;
		SelectChrPage(0, 0 | chrOuter);

		_controlManager = (VsControlManager*)_console->GetControlManager();
	}

	void Reset(bool softReset) override
	{
		BaseMapper::Reset(softReset);
	}

	void Serialize(Serializer& s) override
	{
		BaseMapper::Serialize(s);
		SV(_prgChrSelectBit);
	}

	void ProcessCpuClock() override
	{
		BaseProcessCpuClock();

		if(_controlManager && _prgChrSelectBit != _controlManager->GetPrgChrSelectBit()) {
			_prgChrSelectBit = _controlManager->GetPrgChrSelectBit();

			if(_romInfo.VsType == VsSystemType::Default && _prgSize > 0x8000) {
				//"Note: In case of games with 40KiB PRG - ROM(as found in VS Gumshoe), the above bit additionally changes 8KiB PRG - ROM at $8000 - $9FFF."
				//"Only Vs. Gumshoe uses the 40KiB PRG variant; in the iNES encapsulation, the 8KiB banks are arranged as 0, 1, 2, 3, 0alternate, empty"
				SelectPrgPage(0, _prgChrSelectBit << 2);
			}

			uint8_t chrOuter = _console->IsVsMainConsole() ? 0 : 2;
			SelectChrPage(0, _prgChrSelectBit | chrOuter);
		}
	}
};
