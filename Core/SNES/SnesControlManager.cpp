#include "stdafx.h"
#include "SNES/SnesControlManager.h"
#include "SNES/SnesConsole.h"
#include "SNES/SnesMemoryManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/KeyManager.h"
#include "Shared/Interfaces/IKeyManager.h"
#include "Shared/Interfaces/IInputProvider.h"
#include "Shared/Interfaces/IInputRecorder.h"
#include "SNES/Input/SnesController.h"
#include "SNES/Input/SnesMouse.h"
#include "SNES/Input/Multitap.h"
#include "SNES/Input/SuperScope.h"
#include "EventType.h"
#include "Utilities/Serializer.h"
#include "Shared/SystemActionManager.h"

SnesControlManager::SnesControlManager(SnesConsole* console) : BaseControlManager(console->GetEmulator())
{
	_console = console;
	UpdateControlDevices();
}

SnesControlManager::~SnesControlManager()
{
}

ControllerType SnesControlManager::GetControllerType(uint8_t port)
{
	return _emu->GetSettings()->GetSnesConfig().Controllers[port].Type;
}

shared_ptr<BaseControlDevice> SnesControlManager::CreateControllerDevice(ControllerType type, uint8_t port)
{
	shared_ptr<BaseControlDevice> device;

	SnesConfig cfg = _emu->GetSettings()->GetSnesConfig();

	switch(type) {
		case ControllerType::None: break;
		case ControllerType::SnesController: device.reset(new SnesController(_emu, port, cfg.Controllers[port].Keys)); break;
		case ControllerType::SnesMouse: device.reset(new SnesMouse(_emu, port)); break;
		case ControllerType::SuperScope: device.reset(new SuperScope(_console, port, cfg.Controllers[port].Keys)); break;
		case ControllerType::Multitap: device.reset(new Multitap(_console, port, cfg.Controllers[port].Keys, cfg.Controllers[2].Keys, cfg.Controllers[3].Keys, cfg.Controllers[4].Keys)); break;

		default:
			throw std::runtime_error("Unsupported controller type");
	}

	return device;
}

void SnesControlManager::UpdateControlDevices()
{
	SnesConfig cfg = _emu->GetSettings()->GetSnesConfig();
	if(_emu->GetSettings()->IsEqual(_prevConfig, cfg) && _controlDevices.size() > 0) {
		//Do nothing if configuration is unchanged
		return;
	}

	auto lock = _deviceLock.AcquireSafe();
	ClearDevices();
	for(int i = 0; i < 2; i++) {
		shared_ptr<BaseControlDevice> device = CreateControllerDevice(GetControllerType(i), i);
		if(device) {
			RegisterControlDevice(device);
		}
	}
}

uint8_t SnesControlManager::Read(uint16_t addr)
{
	uint8_t value = _console->GetMemoryManager()->GetOpenBus() & (addr == 0x4016 ? 0xFC : 0xE0);
	for(shared_ptr<BaseControlDevice> &device : _controlDevices) {
		value |= device->ReadRam(addr);
	}

	return value;
}

void SnesControlManager::Write(uint16_t addr, uint8_t value)
{
	for(shared_ptr<BaseControlDevice> &device : _controlDevices) {
		device->WriteRam(addr, value);
	}
}

void SnesControlManager::Serialize(Serializer &s)
{
	SnesConfig cfg = _emu->GetSettings()->GetSnesConfig();
	s.Stream(cfg.Controllers[0].Type, cfg.Controllers[1].Type, cfg.Controllers[2].Type, cfg.Controllers[3].Type, cfg.Controllers[4].Type);
	if(!s.IsSaving()) {
		_emu->GetSettings()->SetSnesConfig(cfg);
		UpdateControlDevices();
	}

	for(shared_ptr<BaseControlDevice> &device : _controlDevices) {
		s.Stream(device.get());
	}
}
