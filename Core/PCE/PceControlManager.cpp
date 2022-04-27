#include "stdafx.h"
#include "PCE/PceControlManager.h"
#include "PCE/Input/PceController.h"
#include "Shared/EmuSettings.h"
#include "Shared/BaseControlDevice.h"
#include "Shared/Emulator.h"

PceControlManager::PceControlManager(Emulator* emu) : BaseControlManager(emu)
{
}

PceControlManagerState& PceControlManager::GetState()
{
	return _state;
}

shared_ptr<BaseControlDevice> PceControlManager::CreateControllerDevice(ControllerType type, uint8_t port)
{
	//TODO
	GameboyConfig cfg = _emu->GetSettings()->GetGameboyConfig();
	shared_ptr<BaseControlDevice> device(new PceController(_emu, port, cfg.Controller.Keys));

	return device;
}

uint8_t PceControlManager::ReadInputPort()
{
	uint8_t result = 0;
	for(shared_ptr<BaseControlDevice>& device : _controlDevices) {
		if(device->IsConnected()) {
			result |= device->ReadRam(0);
		}
	}
	return result;
}

void PceControlManager::WriteInputPort(uint8_t value)
{
	for(shared_ptr<BaseControlDevice>& device : _controlDevices) {
		if(device->IsConnected()) {
			device->WriteRam(0, value);
		}
	}
}

void PceControlManager::UpdateControlDevices()
{
	GameboyConfig cfg = _emu->GetSettings()->GetGameboyConfig();
	if(_emu->GetSettings()->IsEqual(_prevConfig, cfg) && _controlDevices.size() > 0) {
		//Do nothing if configuration is unchanged
		return;
	}

	auto lock = _deviceLock.AcquireSafe();

	ClearDevices();

	shared_ptr<BaseControlDevice> device(CreateControllerDevice(ControllerType::GameboyController, 0));
	if(device) {
		RegisterControlDevice(device);
	}
}