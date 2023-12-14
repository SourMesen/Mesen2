#include "pch.h"
#include "SMS/SmsControlManager.h"
#include "SMS/SmsConsole.h"
#include "SMS/SmsVdp.h"
#include "SMS/Input/SmsController.h"
#include "SMS/Input/SmsLightPhaser.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/KeyManager.h"

SmsControlManager::SmsControlManager(Emulator* emu, SmsConsole* console, SmsVdp* vdp) : BaseControlManager(emu, CpuType::Sms)
{
	_emu = emu;
	_console = console;
	_vdp = vdp;

	//"[Port $3F] defaults to an input-only state (ie. with all the low bits set) on startup."
	_state.ControlPort = 0x0F;
}

shared_ptr<BaseControlDevice> SmsControlManager::CreateControllerDevice(ControllerType type, uint8_t port)
{
	shared_ptr<BaseControlDevice> device;

	SmsConfig& cfg = _emu->GetSettings()->GetSmsConfig();

	KeyMappingSet keys;
	switch(port) {
		default:
		case 0: keys = cfg.Port1.Keys; break;
		case 1: keys = cfg.Port2.Keys; break;
	}

	switch(type) {
		default:
		case ControllerType::None: break;
		case ControllerType::SmsController: device.reset(new SmsController(_emu, port, keys)); break;
		case ControllerType::SmsLightPhaser: device.reset(new SmsLightPhaser(_console, port, keys)); break;
	}

	return device;
}

void SmsControlManager::UpdateControlDevices()
{
	SmsConfig& cfg = _emu->GetSettings()->GetSmsConfig();
	if(_emu->GetSettings()->IsEqual(_prevConfig, cfg) && _controlDevices.size() > 0) {
		//Do nothing if configuration is unchanged
		return;
	}

	auto lock = _deviceLock.AcquireSafe();

	ClearDevices();

	for(int i = 0; i < 2; i++) {
		shared_ptr<BaseControlDevice> device = CreateControllerDevice(i == 0 ? cfg.Port1.Type : cfg.Port2.Type, i);
		if(device) {
			RegisterControlDevice(device);
		}
	}
}

bool SmsControlManager::IsPausePressed()
{
	shared_ptr<BaseControlDevice> device = GetControlDevice(0);
	return device && device->IsPressed(SmsController::Buttons::Pause);
}

uint8_t SmsControlManager::InternalReadPort(uint8_t port)
{
	uint8_t value = 0xFF;
	for(shared_ptr<BaseControlDevice>& device : _controlDevices) {
		if(device->IsConnected()) {
			value &= device->ReadRam(port);
		}
	}
	return value;
}

uint8_t SmsControlManager::ReadPort(uint8_t port)
{
	uint8_t value = InternalReadPort(port);

	//Set TR/TH based on the $3F config
	if(port == 0) {
		value &= ~0x20;
		value |= GetTr(false) ? 0x20 : 0;
	} else {
		value &= ~0xC8;
		//TODOSMS add UI option for japan vs overseas model
		value |= GetTh(true) ? 0x80 : 0;
		value |= GetTh(false) ? 0x40 : 0;
		value |= GetTr(true) ? 0x08 : 0;
	}

	return value;
}

bool SmsControlManager::GetTh(bool portB)
{
	if(portB) {
		if(_state.ControlPort & 0x08) {
			return (InternalReadPort(1) & 0x80) != 0;
		} else {
			return (_state.ControlPort & 0x80) != 0;
		}
	} else {
		if(_state.ControlPort & 0x02) {
			return (InternalReadPort(1) & 0x40) != 0;
		} else {
			return (_state.ControlPort & 0x20) != 0;
		}
	}
}

bool SmsControlManager::GetTr(bool portB)
{
	if(portB) {
		if(_state.ControlPort & 0x04) {
			return (InternalReadPort(1) & 0x08) != 0;
		} else {
			return (_state.ControlPort & 0x40) != 0;
		}
	} else {
		if(_state.ControlPort & 0x01) {
			return (InternalReadPort(0) & 0x20) != 0;
		} else {
			return (_state.ControlPort & 0x10) != 0;
		}
	}
}

void SmsControlManager::WriteControlPort(uint8_t value)
{
	uint8_t thA = GetTh(false);
	uint8_t thB = GetTh(true);
	_state.ControlPort = value;
	uint8_t newThA = GetTh(false);
	uint8_t newThB = GetTh(true);
	if((!thA && newThA) || (!thB && newThB)) {
		_vdp->LatchHorizontalCounter();
	}
}

void SmsControlManager::Serialize(Serializer& s)
{
	BaseControlManager::Serialize(s);
	SV(_state.ControlPort);

	if(!s.IsSaving()) {
		UpdateControlDevices();
	}

	for(uint8_t i = 0; i < _controlDevices.size(); i++) {
		SVI(_controlDevices[i]);
	}
}
