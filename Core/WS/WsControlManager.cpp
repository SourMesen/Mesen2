#include "pch.h"
#include "WS/WsConsole.h"
#include "WS/WsControlManager.h"
#include "WS/WsMemoryManager.h"
#include "WS/WsController.h"
#include "Shared/KeyManager.h"

WsControlManager::WsControlManager(Emulator* emu, WsConsole* console) : BaseControlManager(emu, CpuType::Ws)
{
	_console = console;
}

shared_ptr<BaseControlDevice> WsControlManager::CreateControllerDevice(ControllerType type, uint8_t port)
{
	shared_ptr<BaseControlDevice> device;

	WsConfig& cfg = _emu->GetSettings()->GetWsConfig();

	switch(type) {
		default:
		case ControllerType::None: break;
		
		case ControllerType::WsController:
		case ControllerType::WsControllerVertical:
			device.reset(new WsController(_emu, _console, port, cfg.ControllerHorizontal.Keys, cfg.ControllerVertical.Keys));
			break;
	}

	return device;
}

void WsControlManager::UpdateControlDevices()
{
	WsConfig cfg = _emu->GetSettings()->GetWsConfig();
	if(_emu->GetSettings()->IsEqual(_prevConfig, cfg) && _controlDevices.size() > 0) {
		//Do nothing if configuration is unchanged
		return;
	}

	auto lock = _deviceLock.AcquireSafe();

	ClearDevices();

	shared_ptr<BaseControlDevice> device(CreateControllerDevice(ControllerType::WsController, 0));
	if(device) {
		RegisterControlDevice(device);
	}
}

uint8_t WsControlManager::Read()
{
	uint8_t result = 0;

	for(shared_ptr<BaseControlDevice>& controller : _controlDevices) {
		if(controller->GetPort() == 0 && controller->GetControllerType() == ControllerType::WsController) {
			if(_state.InputSelect & 0x10) {
				result |= controller->IsPressed(WsController::Up2) ? 0x01 : 0;
				result |= controller->IsPressed(WsController::Right2) ? 0x02 : 0;
				result |= controller->IsPressed(WsController::Down2) ? 0x04 : 0;
				result |= controller->IsPressed(WsController::Left2) ? 0x08 : 0;
			}
			if(_state.InputSelect & 0x20) {
				result |= controller->IsPressed(WsController::Up) ? 0x01 : 0;
				result |= controller->IsPressed(WsController::Right) ? 0x02 : 0;
				result |= controller->IsPressed(WsController::Down) ? 0x04 : 0;
				result |= controller->IsPressed(WsController::Left) ? 0x08 : 0;
			}
			if(_state.InputSelect & 0x40) {
				result |= controller->IsPressed(WsController::Start) ? 0x02 : 0;
				result |= controller->IsPressed(WsController::A) ? 0x04 : 0;
				result |= controller->IsPressed(WsController::B) ? 0x08 : 0;
			}
		}
	}

	return result;
}

void WsControlManager::Write(uint8_t value)
{
	_state.InputSelect = value & 0x70;
}

bool WsControlManager::IsSoundPressed()
{
	for(shared_ptr<BaseControlDevice>& controller : _controlDevices) {
		if(controller->GetPort() == 0 && controller->GetControllerType() == ControllerType::WsController) {
			return controller->IsPressed(WsController::Sound);
		}
	}
	return false;
}

void WsControlManager::UpdateInputState()
{
	BaseControlManager::UpdateInputState();
	
	uint8_t newInput = Read();
	if((_prevInput | newInput) > _prevInput) {
		//Trigger IRQ whenever any extra bits are set compared to the previous input
		_console->GetMemoryManager()->SetIrqSource(WsIrqSource::KeyPressed);
	}
	_prevInput = newInput;

	bool soundButtonPressed = IsSoundPressed();
	if(soundButtonPressed && !_soundButtonPressed) {
		_console->GetApu()->ChangeMasterVolume();
	}
	_soundButtonPressed = soundButtonPressed;
}

void WsControlManager::Serialize(Serializer& s)
{
	SV(_state.InputSelect);
	SV(_prevInput);
	SV(_soundButtonPressed);
}
