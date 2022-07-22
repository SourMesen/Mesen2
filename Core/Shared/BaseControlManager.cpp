#include "stdafx.h"
#include "Shared/BaseControlManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/KeyManager.h"
#include "Shared/ControllerHub.h"
#include "Shared/Interfaces/IKeyManager.h"
#include "Shared/Interfaces/IInputProvider.h"
#include "Shared/Interfaces/IInputRecorder.h"
#include "SystemActionManager.h"
#include "EventType.h"
#include "Utilities/Serializer.h"
#include "Shared/MessageManager.h"

BaseControlManager::BaseControlManager(Emulator* emu)
{
	_emu = emu;
	_pollCounter = 0;
	AddSystemControlDevice(_emu->GetSystemActionManager());
	UpdateControlDevices();
}

BaseControlManager::~BaseControlManager()
{
}

void BaseControlManager::AddSystemControlDevice(shared_ptr<BaseControlDevice> device)
{
	_controlDevices.clear();
	_systemDevices.push_back(device);
	UpdateControlDevices();
}

void BaseControlManager::RegisterInputProvider(IInputProvider* provider)
{
	auto lock = _deviceLock.AcquireSafe();
	_inputProviders.push_back(provider);
}

void BaseControlManager::UnregisterInputProvider(IInputProvider* provider)
{
	auto lock = _deviceLock.AcquireSafe();
	vector<IInputProvider*>& vec = _inputProviders;
	vec.erase(std::remove(vec.begin(), vec.end(), provider), vec.end());
}

void BaseControlManager::RegisterInputRecorder(IInputRecorder* provider)
{
	auto lock = _deviceLock.AcquireSafe();
	_inputRecorders.push_back(provider);
}

void BaseControlManager::UnregisterInputRecorder(IInputRecorder* provider)
{
	auto lock = _deviceLock.AcquireSafe();
	vector<IInputRecorder*>& vec = _inputRecorders;
	vec.erase(std::remove(vec.begin(), vec.end(), provider), vec.end());
}

vector<ControllerData> BaseControlManager::GetPortStates()
{
	vector<ControllerData> states;
	for(shared_ptr<BaseControlDevice>& device : _controlDevices) {
		states.push_back({
			device->GetControllerType(),
			device->GetRawState(),
			device->GetPort()
		});
	}
	return states;
}

shared_ptr<BaseControlDevice> BaseControlManager::GetControlDevice(uint8_t port)
{
	auto lock = _deviceLock.AcquireSafe();

	auto result = std::find_if(_controlDevices.begin(), _controlDevices.end(), [port](const shared_ptr<BaseControlDevice> control) { return control->GetPort() == port; });
	if(result != _controlDevices.end()) {
		return *result;
	}
	return nullptr;
}

shared_ptr<BaseControlDevice> BaseControlManager::GetControlDeviceByIndex(uint8_t index)
{
	auto lock = _deviceLock.AcquireSafe();

	int counter = 0;
	for(size_t i = 0; i < _controlDevices.size(); i++) {
		if(_controlDevices[i] && _controlDevices[i]->GetPort() <= 2) {
			shared_ptr<IControllerHub> hub = std::dynamic_pointer_cast<IControllerHub>(_controlDevices[i]);
			if(hub) {
				int portCount = hub->GetHubPortCount();
				for(int j = 0; j < portCount; j++) {
					shared_ptr<BaseControlDevice> device = hub->GetController(j);
					if(counter == index) {
						return device;
					}
					counter++;
				}
			} else {
				if(counter == index) {
					return _controlDevices[i];
				}
				counter++;
			}
		}
	}
	return nullptr;
}

void BaseControlManager::RefreshHubState()
{
	auto lock = _deviceLock.AcquireSafe();

	for(size_t i = 0; i < _controlDevices.size(); i++) {
		if(_controlDevices[i] && _controlDevices[i]->GetPort() <= 2) {
			shared_ptr<IControllerHub> hub = std::dynamic_pointer_cast<IControllerHub>(_controlDevices[i]);
			if(hub) {
				hub->RefreshHubState();
			}
		}
	}
}

vector<shared_ptr<BaseControlDevice>> BaseControlManager::GetControlDevices()
{
	return _controlDevices;
}

void BaseControlManager::RegisterControlDevice(shared_ptr<BaseControlDevice> controlDevice)
{
	_controlDevices.push_back(controlDevice);
}

void BaseControlManager::ClearDevices()
{
	_controlDevices.clear();

	for(shared_ptr<BaseControlDevice> device : _systemDevices) {
		RegisterControlDevice(device);
	}
}

void BaseControlManager::UpdateInputState()
{
	KeyManager::RefreshKeyState();

	auto lock = _deviceLock.AcquireSafe();

	//string log = "F: " + std::to_string(_emu->GetFrameCount()) + " C:" + std::to_string(_pollCounter) + " ";
	for(shared_ptr<BaseControlDevice>& device : _controlDevices) {
		device->ClearState();
		device->SetStateFromInput();

		for(size_t i = 0; i < _inputProviders.size(); i++) {
			IInputProvider* provider = _inputProviders[i];
			if(provider->SetInput(device.get())) {
				break;
			}
		}

		device->OnAfterSetState();
		//log += "|" + device->GetTextState();
	}

	_emu->ProcessEvent(EventType::InputPolled);

	if(!_emu->IsRunAheadFrame()) {
		for(IInputRecorder* recorder : _inputRecorders) {
			recorder->RecordInput(_controlDevices);
		}
	}

	//MessageManager::Log(log);

	_pollCounter++;
}

void BaseControlManager::ProcessEndOfFrame()
{
	if(!_wasInputRead) {
		_lagCounter++;
	}
	_wasInputRead = false;
}

void BaseControlManager::SetInputReadFlag()
{
	//Used for lag counter - any frame where the input is read does not count as lag
	_wasInputRead = true;
}

uint32_t BaseControlManager::GetLagCounter()
{
	return _lagCounter;
}

void BaseControlManager::ResetLagCounter()
{
	_lagCounter = 0;
}

bool BaseControlManager::HasControlDevice(ControllerType type)
{
	auto lock = _deviceLock.AcquireSafe();

	for(shared_ptr<BaseControlDevice>& device : _controlDevices) {
		if(device->HasControllerType(type)) {
			return true;
		}
	}
	return false;
}

uint32_t BaseControlManager::GetPollCounter()
{
	return BaseControlManager::_pollCounter;
}

void BaseControlManager::SetPollCounter(uint32_t value)
{
	_pollCounter = value;
}
