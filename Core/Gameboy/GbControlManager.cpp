#include "pch.h"
#include "Gameboy/Gameboy.h"
#include "Gameboy/GbMemoryManager.h"
#include "Gameboy/GbControlManager.h"
#include "Gameboy/Input/GbController.h"
#include "SNES/Coprocessors/SGB/SuperGameboy.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/KeyManager.h"
#include "Shared/SystemActionManager.h"
#include <functional>

GbControlManager::GbControlManager(Emulator* emu, Gameboy* console) : BaseControlManager(emu, CpuType::Gameboy)
{
	_emu = emu;
	_console = console;
}

GbControlManagerState GbControlManager::GetState()
{
	return _state;
}

shared_ptr<BaseControlDevice> GbControlManager::CreateControllerDevice(ControllerType type, uint8_t port)
{
	if(_console->IsSgb()) {
		return nullptr;
	}

	shared_ptr<BaseControlDevice> device;

	GameboyConfig& cfg = _emu->GetSettings()->GetGameboyConfig();

	switch(type) {
		default:
		case ControllerType::None: break;

		case ControllerType::GameboyController: device.reset(new GbController(_emu, port, cfg.Controller.Keys)); break;
	}

	return device;
}

void GbControlManager::UpdateControlDevices()
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

uint8_t GbControlManager::ReadInputPort()
{
	//Bit 7 - Not used
	//Bit 6 - Not used
	//Bit 5 - P15 Select Button Keys      (0=Select)
	//Bit 4 - P14 Select Direction Keys   (0=Select)
	//Bit 3 - P13 Input Down  or Start    (0=Pressed) (Read Only)
	//Bit 2 - P12 Input Up    or Select   (0=Pressed) (Read Only)
	//Bit 1 - P11 Input Left  or Button B (0=Pressed) (Read Only)
	//Bit 0 - P10 Input Right or Button A (0=Pressed) (Read Only)
	uint8_t result = 0x0F;

	uint8_t inputSelect = _state.InputSelect;
	SuperGameboy* sgb = _console->GetSgb();
	if(sgb) {
		if((inputSelect & 0x30) == 0x30) {
			result = sgb->GetInputIndex();
		} else {
			if(!(inputSelect & 0x20)) {
				result &= sgb->GetInput() >> 4;
			}
			if(!(inputSelect & 0x10)) {
				result &= sgb->GetInput() & 0x0F;
			}
		}
	} else {
		for(shared_ptr<BaseControlDevice>& controller : _controlDevices) {
			if(controller->GetPort() == 0 && controller->GetControllerType() == ControllerType::GameboyController) {
				if(!(inputSelect & 0x20)) {
					result &= ~(controller->IsPressed(GbController::A) ? 0x01 : 0);
					result &= ~(controller->IsPressed(GbController::B) ? 0x02 : 0);
					result &= ~(controller->IsPressed(GbController::Select) ? 0x04 : 0);
					result &= ~(controller->IsPressed(GbController::Start) ? 0x08 : 0);
				}
				if(!(inputSelect & 0x10)) {
					result &= ~(controller->IsPressed(GbController::Right) ? 0x01 : 0);
					result &= ~(controller->IsPressed(GbController::Left) ? 0x02 : 0);
					result &= ~(controller->IsPressed(GbController::Up) ? 0x04 : 0);
					result &= ~(controller->IsPressed(GbController::Down) ? 0x08 : 0);
				}
			}
		}
	}

	return result | (inputSelect & 0x30) | 0xC0;
}

void GbControlManager::WriteInputPort(uint8_t value)
{
	//Changing the select bits can trigger the joypad IRQ (Fixes Double Dragon 3 input issues)
	ProcessInputChange([&]() { _state.InputSelect = value & 0x30; });

	SuperGameboy* sgb = _console->GetSgb();
	if(sgb) {
		sgb->ProcessInputPortWrite(_state.InputSelect);
	}
}

void GbControlManager::ProcessInputChange(std::function<void()> inputUpdateCallback)
{
	uint8_t prevInput = ReadInputPort();
	inputUpdateCallback();
	uint8_t newInput = ReadInputPort();
	if(prevInput != newInput) {
		_console->GetMemoryManager()->RequestIrq(GbIrqSource::Joypad);
	}
}

void GbControlManager::UpdateInputState()
{
	ProcessInputChange([this]() { BaseControlManager::UpdateInputState(); });
}

void GbControlManager::Serialize(Serializer& s)
{
	BaseControlManager::Serialize(s);

	SV(_state.InputSelect);
	for(uint8_t i = 0; i < _controlDevices.size(); i++) {
		SVI(_controlDevices[i]);
	}
}
