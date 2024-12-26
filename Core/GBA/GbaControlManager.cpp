#include "pch.h"
#include "GBA/GbaControlManager.h"
#include "GBA/GbaMemoryManager.h"
#include "GBA/Input/GbaController.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Utilities/BitUtilities.h"

GbaControlManager::GbaControlManager(Emulator* emu, GbaConsole* console) : BaseControlManager(emu, CpuType::Gba)
{
	_console = console;
}

void GbaControlManager::Init(GbaMemoryManager* memoryManager)
{
	_memoryManager = memoryManager;
}

GbaControlManagerState& GbaControlManager::GetState()
{
	return _state;
}

void GbaControlManager::UpdateInputState()
{
	BaseControlManager::UpdateInputState();
	_state.ActiveKeys = (ReadController(0) | (ReadController(1) << 8));
	CheckForIrq();
}

shared_ptr<BaseControlDevice> GbaControlManager::CreateControllerDevice(ControllerType type, uint8_t port)
{
	shared_ptr<BaseControlDevice> device;

	GbaConfig& cfg = _emu->GetSettings()->GetGbaConfig();

	switch(type) {
		default:
		case ControllerType::None: break;

		case ControllerType::GbaController: device.reset(new GbaController(_emu, port, cfg.Controller.Keys)); break;
	}

	return device;
}

void GbaControlManager::UpdateControlDevices()
{
	GbaConfig cfg = _emu->GetSettings()->GetGbaConfig();
	if(_emu->GetSettings()->IsEqual(_prevConfig, cfg) && _controlDevices.size() > 0) {
		//Do nothing if configuration is unchanged
		return;
	}

	auto lock = _deviceLock.AcquireSafe();

	ClearDevices();

	shared_ptr<BaseControlDevice> device(CreateControllerDevice(ControllerType::GbaController, 0));
	if(device) {
		RegisterControlDevice(device);
	}
}

uint8_t GbaControlManager::ReadController(uint32_t addr)
{
	for(shared_ptr<BaseControlDevice>& controller : _controlDevices) {
		if(controller->GetPort() == 0 && controller->GetControllerType() == ControllerType::GbaController) {
			if(addr & 0x01) {
				//TODOGBA other bits are 0 or 1?
				return (
					(controller->IsPressed(GbaController::R) ? 0 : (1 << 0)) |
					(controller->IsPressed(GbaController::L) ? 0 : (1 << 1))
				);
			} else {
				return (
					(controller->IsPressed(GbaController::A) ? 0 : (1 << 0)) |
					(controller->IsPressed(GbaController::B) ? 0 : (1 << 1)) |
					(controller->IsPressed(GbaController::Select) ? 0 : (1 << 2)) |
					(controller->IsPressed(GbaController::Start) ? 0 : (1 << 3)) |
					(controller->IsPressed(GbaController::Right) ? 0 : (1 << 4)) |
					(controller->IsPressed(GbaController::Left) ? 0 : (1 << 5)) |
					(controller->IsPressed(GbaController::Up) ? 0 : (1 << 6)) |
					(controller->IsPressed(GbaController::Down) ? 0 : (1 << 7))
				);
			}
		}
	}

	return 0xFF;
}

uint8_t GbaControlManager::ReadInputPort(uint32_t addr)
{
	if(addr & 0x02) {
		if(addr & 0x01) {
			return BitUtilities::GetBits<0>(_state.KeyControl);
		} else {
			return BitUtilities::GetBits<8>(_state.KeyControl);
		}
	} else {
		SetInputReadFlag();
		return (addr & 0x01) ? (_state.ActiveKeys >> 8) : (uint8_t)_state.ActiveKeys;
	}
}

bool GbaControlManager::CheckInputCondition()
{
	uint16_t activeKeys = _state.ActiveKeys ^ 0x3FF;
	if(_state.KeyControl & 0x8000) {
		//AND mode
		if((activeKeys & 0x3FF) == (_state.KeyControl & 0x3FF)) {
			return true;
		}
	} else {
		//OR mode
		if(activeKeys & _state.KeyControl & 0x3FF) {
			return true;
		}
	}
	return false;
}

void GbaControlManager::CheckForIrq()
{
	if(_state.KeyControl & 0x4000) {
		if(CheckInputCondition()) {
			_memoryManager->SetIrqSource(GbaIrqSource::Keypad);
		}
	}
}

void GbaControlManager::WriteInputPort(GbaAccessModeVal mode, uint32_t addr, uint8_t value)
{
	if(addr & 0x01) {
		BitUtilities::SetBits<8>(_state.KeyControl, value);
	} else {
		BitUtilities::SetBits<0>(_state.KeyControl, value);
	}

	bool updateIrq = (addr & 0x03) == 0x03 || (mode & GbaAccessMode::Byte) || ((mode & GbaAccessMode::HalfWord) && (addr & 0x01));
	if(updateIrq) {
		//Only check for irq updates after the write operation is done (otherwise this could trigger an unexpected IRQ)
		CheckForIrq();
	}
}

void GbaControlManager::Serialize(Serializer& s)
{
	BaseControlManager::Serialize(s);
	SV(_state.KeyControl);
	SV(_state.ActiveKeys);
	for(uint8_t i = 0; i < _controlDevices.size(); i++) {
		SVI(_controlDevices[i]);
	}
}