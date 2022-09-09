#include "pch.h"
#include "NES/Mappers/FDS/FdsInputButtons.h"
#include "NES/Mappers/FDS/Fds.h"
#include "Shared/Emulator.h"
#include "Shared/MessageManager.h"
#include "Shared/BaseControlDevice.h"
#include "Utilities/Serializer.h"

FdsInputButtons::FdsInputButtons(Fds* fds, Emulator* emu) : BaseControlDevice(emu, ControllerType::None, BaseControlDevice::MapperInputPort)
{
	_fds = fds;
	_emu = emu;
	_sideCount = fds->GetSideCount();
}

string FdsInputButtons::GetKeyNames()
{
	return string("E0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ").substr(0, _sideCount + 1);
}

void FdsInputButtons::Serialize(Serializer& s)
{
	BaseControlDevice::Serialize(s);
	SV(_needEjectDisk); SV(_insertDiskNumber); SV(_insertDiskDelay);
}

void FdsInputButtons::OnAfterSetState()
{
	if(_needEjectDisk) {
		SetBit(FdsInputButtons::FdsButtons::EjectDiskButton);
		_needEjectDisk = false;
	}
	if(_insertDiskDelay > 0) {
		_insertDiskDelay--;
		if(_insertDiskDelay == 0) {
			SetBit(FdsInputButtons::FdsButtons::InsertDisk1 + _insertDiskNumber);
		}
	}

	bool needEject = IsPressed(FdsInputButtons::FdsButtons::EjectDiskButton);
	int diskToInsert = -1;
	for(int i = 0; i < 16; i++) {
		if(IsPressed(FdsInputButtons::FdsButtons::InsertDisk1 + i)) {
			diskToInsert = i;
			break;
		}
	}

	if(needEject || diskToInsert >= 0) {
		if(needEject) {
			_fds->EjectDisk();
		}

		if(diskToInsert >= 0) {
			_fds->InsertDisk(diskToInsert);
		}
	}
}

void FdsInputButtons::EjectDisk()
{
	_needEjectDisk = true;
}

void FdsInputButtons::InsertDisk(uint8_t diskNumber)
{
	if(diskNumber >= _sideCount) {
		return;
	}

	if(_fds->IsDiskInserted()) {
		//Eject disk on next frame, then insert new disk 2 seconds later
		_needEjectDisk = true;
		_insertDiskNumber = diskNumber;
		_insertDiskDelay = FdsInputButtons::ReinsertDiskFrameDelay;
	} else {
		//Insert disk on next frame
		_insertDiskNumber = diskNumber;
		_insertDiskDelay = 1;
	}
	MessageManager::DisplayMessage("FDS", "FdsDiskInserted", std::to_string(diskNumber / 2 + 1), diskNumber & 0x01 ? "B" : "A");
}

void FdsInputButtons::SwitchDiskSide()
{
	if(!_fds->IsAutoInsertDiskEnabled()) {
		if(_fds->IsDiskInserted()) {
			_emu->Pause();
			InsertDisk((_fds->GetCurrentDisk() ^ 0x01) % _fds->GetSideCount());
			_emu->Resume();
		}
	}
}

void FdsInputButtons::InsertNextDisk()
{
	if(!_fds->IsAutoInsertDiskEnabled()) {
		_emu->Pause();
		InsertDisk(((_fds->GetCurrentDisk() & 0xFE) + 2) % _fds->GetSideCount());
		_emu->Resume();
	}
}

uint8_t FdsInputButtons::ReadRam(uint16_t addr)
{
	return 0;
}

void FdsInputButtons::WriteRam(uint16_t addr, uint8_t value)
{
}

void FdsInputButtons::ProcessNotification(ConsoleNotificationType type, void* parameter)
{
	if(type == ConsoleNotificationType::ExecuteShortcut) {
		ExecuteShortcutParams* params = (ExecuteShortcutParams*)parameter;
		switch(params->Shortcut) {
			default: break;
			case EmulatorShortcut::FdsEjectDisk: EjectDisk(); break;
			case EmulatorShortcut::FdsSwitchDiskSide: SwitchDiskSide(); break;
			case EmulatorShortcut::FdsInsertNextDisk: InsertNextDisk(); break;
			case EmulatorShortcut::FdsInsertDiskNumber: InsertDisk(params->Param); break;
		}
	}
}
