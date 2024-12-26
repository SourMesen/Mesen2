#include "pch.h"
#include "SMS/SmsControlManager.h"
#include "SMS/SmsConsole.h"
#include "SMS/SmsVdp.h"
#include "SMS/Input/SmsController.h"
#include "SMS/Input/SmsLightPhaser.h"
#include "SMS/Input/ColecoVisionController.h"
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

	bool isCv = _console->GetModel() == SmsModel::ColecoVision;

	KeyMappingSet keys;
	switch(port) {
		default:
		case 0: keys = isCv ? _emu->GetSettings()->GetCvConfig().Port1.Keys : _emu->GetSettings()->GetSmsConfig().Port1.Keys; break;
		case 1: keys = isCv ? _emu->GetSettings()->GetCvConfig().Port2.Keys : _emu->GetSettings()->GetSmsConfig().Port2.Keys; break;
	}

	switch(type) {
		default:
		case ControllerType::None: break;
		case ControllerType::SmsController: device.reset(new SmsController(_emu, port, keys)); break;
		case ControllerType::SmsLightPhaser: device.reset(new SmsLightPhaser(_console, port, keys)); break;
		case ControllerType::ColecoVisionController: device.reset(new ColecoVisionController(_emu, port, keys)); break;
	}

	return device;
}

void SmsControlManager::UpdateControlDevices()
{
	SmsConfig& cfg = _emu->GetSettings()->GetSmsConfig();
	CvConfig& cvCfg = _emu->GetSettings()->GetCvConfig();
	if(_emu->GetSettings()->IsEqual(_prevConfig, cfg) && _emu->GetSettings()->IsEqual(_prevCvConfig, cvCfg) && _controlDevices.size() > 0) {
		//Do nothing if configuration is unchanged
		return;
	}

	auto lock = _deviceLock.AcquireSafe();

	ClearDevices();

	bool isCv = _console->GetModel() == SmsModel::ColecoVision;
	for(int i = 0; i < 2; i++) {
		shared_ptr<BaseControlDevice> device;
		if(isCv) {
			device = CreateControllerDevice(i == 0 ? cvCfg.Port1.Type : cvCfg.Port2.Type, i);
		} else {
			device = CreateControllerDevice(i == 0 ? cfg.Port1.Type : cfg.Port2.Type, i);
		}
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
	SetInputReadFlag();

	if(_console->GetModel() == SmsModel::ColecoVision) {
		return ReadColecoVisionPort(port);
	}

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
	if(_console->GetModel() == SmsModel::ColecoVision) {
		WriteColecoVisionPort(value);
	} else {
		uint8_t thA = GetTh(false);
		uint8_t thB = GetTh(true);
		_state.ControlPort = value;
		uint8_t newThA = GetTh(false);
		uint8_t newThB = GetTh(true);
		if((!thA && newThA) || (!thB && newThB)) {
			_vdp->LatchHorizontalCounter();
		}
	}
}

uint8_t SmsControlManager::ReadColecoVisionPort(uint8_t port)
{
	for(shared_ptr<BaseControlDevice>& device : _controlDevices) {
		if(device->IsConnected() && device->GetPort() == port) {
			return device->ReadRam(port);
		}
	}
	return 0xFF;
}

void SmsControlManager::WriteColecoVisionPort(uint8_t value)
{
	for(shared_ptr<BaseControlDevice>& device : _controlDevices) {
		if(device->IsConnected()) {
			device->WriteRam(0, value);
		}
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
