#pragma once
#include "pch.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Interfaces/INotificationListener.h"
#include "Utilities/ISerializable.h"

class Emulator;
class Fds;

class FdsInputButtons : public BaseControlDevice, public INotificationListener
{
private:
	const uint8_t ReinsertDiskFrameDelay = 120;

	Fds* _fds = nullptr;
	Emulator* _emu = nullptr;

	bool _needEjectDisk = false;
	uint8_t _insertDiskNumber = 0;
	uint8_t _insertDiskDelay = 0;
	uint32_t _sideCount = 0;

protected:
	string GetKeyNames() override;

	void Serialize(Serializer& s) override;

	void EjectDisk();
	void SwitchDiskSide();
	void InsertNextDisk();

public:
	enum FdsButtons { EjectDiskButton = 0, InsertDisk1 };

	FdsInputButtons(Fds* fds, Emulator* emu);

	void OnAfterSetState() override;
	
	void InsertDisk(uint8_t diskNumber);

	uint8_t ReadRam(uint16_t addr) override;
	void WriteRam(uint16_t addr, uint8_t value) override;

	void ProcessNotification(ConsoleNotificationType type, void* parameter) override;
};
