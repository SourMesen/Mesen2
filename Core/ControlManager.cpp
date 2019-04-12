#include "stdafx.h"
#include "ControlManager.h"
#include "Console.h"
#include "EmuSettings.h"
#include "MemoryManager.h"
#include "KeyManager.h"
#include "IKeyManager.h"
#include "IInputProvider.h"
#include "IInputRecorder.h"
#include "SystemActionManager.h"
#include "SnesController.h"
#include "SnesMouse.h"
#include "../Utilities/Serializer.h"

ControlManager::ControlManager(shared_ptr<Console> console)
{
	_console = console;
	_inputConfigVersion = -1;
	_pollCounter = 0;
	_systemActionManager.reset(new SystemActionManager(console));
	UpdateControlDevices();
}

ControlManager::~ControlManager()
{
}

void ControlManager::RegisterInputProvider(IInputProvider* provider)
{
	auto lock = _deviceLock.AcquireSafe();
	_inputProviders.push_back(provider);
}

void ControlManager::UnregisterInputProvider(IInputProvider* provider)
{
	auto lock = _deviceLock.AcquireSafe();
	vector<IInputProvider*> &vec = _inputProviders;
	vec.erase(std::remove(vec.begin(), vec.end(), provider), vec.end());
}

void ControlManager::RegisterInputRecorder(IInputRecorder* provider)
{
	auto lock = _deviceLock.AcquireSafe();
	_inputRecorders.push_back(provider);
}

void ControlManager::UnregisterInputRecorder(IInputRecorder* provider)
{
	auto lock = _deviceLock.AcquireSafe();
	vector<IInputRecorder*> &vec = _inputRecorders;
	vec.erase(std::remove(vec.begin(), vec.end(), provider), vec.end());
}

vector<ControlDeviceState> ControlManager::GetPortStates()
{
	auto lock = _deviceLock.AcquireSafe();

	vector<ControlDeviceState> states;
	for(int i = 0; i < 4; i++) {
		shared_ptr<BaseControlDevice> device = GetControlDevice(i);
		if(device) {
			states.push_back(device->GetRawState());
		} else {
			states.push_back(ControlDeviceState());
		}
	}
	return states;
}

shared_ptr<SystemActionManager> ControlManager::GetSystemActionManager()
{
	return _systemActionManager;
}

shared_ptr<BaseControlDevice> ControlManager::GetControlDevice(uint8_t port)
{
	auto lock = _deviceLock.AcquireSafe();

	auto result = std::find_if(_controlDevices.begin(), _controlDevices.end(), [port](const shared_ptr<BaseControlDevice> control) { return control->GetPort() == port; });
	if(result != _controlDevices.end()) {
		return *result;
	}
	return nullptr;
}

vector<shared_ptr<BaseControlDevice>> ControlManager::GetControlDevices()
{
	return _controlDevices;
}

void ControlManager::RegisterControlDevice(shared_ptr<BaseControlDevice> controlDevice)
{
	_controlDevices.push_back(controlDevice);
}

ControllerType ControlManager::GetControllerType(uint8_t port)
{
	return _console->GetSettings()->GetInputConfig().Controllers[port].Type;
}

shared_ptr<BaseControlDevice> ControlManager::CreateControllerDevice(ControllerType type, uint8_t port, shared_ptr<Console> console)
{
	shared_ptr<BaseControlDevice> device;
	
	InputConfig cfg = console->GetSettings()->GetInputConfig();

	switch(type) {
		case ControllerType::None: break;
		case ControllerType::SnesController: device.reset(new SnesController(console, port, cfg.Controllers[port].Keys)); break;
		case ControllerType::SnesMouse: device.reset(new SnesMouse(console, port)); break;
		case ControllerType::SuperScope: break;
	}
	
	return device;
}

void ControlManager::UpdateControlDevices()
{
	uint32_t version = _console->GetSettings()->GetInputConfigVersion();
	if(_inputConfigVersion != version) {
		_inputConfigVersion = version;

		auto lock = _deviceLock.AcquireSafe();
		_controlDevices.clear();
		RegisterControlDevice(_systemActionManager);
		for(int i = 0; i < 4; i++) {
			shared_ptr<BaseControlDevice> device = CreateControllerDevice(GetControllerType(i), i, _console);
			if(device) {
				RegisterControlDevice(device);
			}
		}
	}
	_systemActionManager->ProcessSystemActions();
}

void ControlManager::UpdateInputState()
{
	KeyManager::RefreshKeyState();

	auto lock = _deviceLock.AcquireSafe();

	string log = "";
	for(shared_ptr<BaseControlDevice> &device : _controlDevices) {
		device->ClearState();

		bool inputSet = false;
		for(size_t i = 0; i < _inputProviders.size(); i++) {
			IInputProvider* provider = _inputProviders[i];
			if(provider->SetInput(device.get())) {
				inputSet = true;
				break;
			}
		}

		if(!inputSet) {
			device->SetStateFromInput();
		}

		device->OnAfterSetState();
		//log += "|" + device->GetTextState();
	}

	//TODO
	/*
	shared_ptr<Debugger> debugger = _console->GetDebugger(false);
	if(debugger) {
		debugger->ProcessEvent(EventType::InputPolled);
	}*/

	for(IInputRecorder* recorder : _inputRecorders) {
		recorder->RecordInput(_controlDevices);
	}

	//MessageManager::Log(log);

	_pollCounter++;
}

uint32_t ControlManager::GetPollCounter()
{
	return ControlManager::_pollCounter;
}

void ControlManager::SetPollCounter(uint32_t value)
{
	_pollCounter = value;
}

uint8_t ControlManager::Read(uint16_t addr)
{
	uint8_t value = _console->GetMemoryManager()->GetOpenBus() & (addr == 0x4016 ? 0xFC : 0xE0);
	for(shared_ptr<BaseControlDevice> &device : _controlDevices) {
		value |= device->ReadRam(addr);
	}

	return value;
}

void ControlManager::Write(uint16_t addr, uint8_t value)
{
	for(shared_ptr<BaseControlDevice> &device : _controlDevices) {
		device->WriteRam(addr, value);
	}
}

void ControlManager::Serialize(Serializer &s)
{
	InputConfig cfg = _console->GetSettings()->GetInputConfig();
	s.Stream(cfg.Controllers[0].Type, cfg.Controllers[1].Type, cfg.Controllers[2].Type, cfg.Controllers[3].Type);
	if(!s.IsSaving()) {
		_console->GetSettings()->SetInputConfig(cfg);
		UpdateControlDevices();
	}

	for(shared_ptr<BaseControlDevice> &device : _controlDevices) {
		s.Stream(device.get());
	}
}
