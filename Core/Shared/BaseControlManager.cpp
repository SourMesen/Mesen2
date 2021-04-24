#include "stdafx.h"
#include "Shared/BaseControlManager.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/KeyManager.h"
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
	auto lock = _deviceLock.AcquireSafe();

	vector<ControllerData> states;
	for(int i = 0; i < 2; i++) {
		shared_ptr<BaseControlDevice> device = GetControlDevice(i);
		if(device) {
			states.push_back({ device->GetControllerType(), device->GetRawState() });
		} else {
			states.push_back({ ControllerType::None, ControlDeviceState() });
		}
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

	shared_ptr<Debugger> debugger = _emu->GetDebugger(false);
	if(debugger) {
		debugger->ProcessEvent(EventType::InputPolled);
	}

	if(!_emu->IsRunAheadFrame()) {
		for(IInputRecorder* recorder : _inputRecorders) {
			recorder->RecordInput(_controlDevices);
		}
	}

	//MessageManager::Log(log);

	_pollCounter++;
}

uint32_t BaseControlManager::GetPollCounter()
{
	return BaseControlManager::_pollCounter;
}

void BaseControlManager::SetPollCounter(uint32_t value)
{
	_pollCounter = value;
}

