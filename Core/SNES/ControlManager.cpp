#include "stdafx.h"
#include "ControlManager.h"
#include "Console.h"
#include "Emulator.h"
#include "EmuSettings.h"
#include "MemoryManager.h"
#include "KeyManager.h"
#include "IKeyManager.h"
#include "IInputProvider.h"
#include "IInputRecorder.h"
#include "SystemActionManager.h"
#include "SnesController.h"
#include "SnesMouse.h"
#include "Multitap.h"
#include "SuperScope.h"
#include "EventType.h"
#include "../Utilities/Serializer.h"

ControlManager::ControlManager(Console* console)
{
	_console = console;
	_emu = _console->GetEmulator();
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

vector<ControllerData> ControlManager::GetPortStates()
{
	auto lock = _deviceLock.AcquireSafe();

	vector<ControllerData> states;
	for(int i = 0; i < 2; i++) {
		shared_ptr<BaseControlDevice> device = GetControlDevice(i);
		if(device) {
			states.push_back({ device->GetControllerType(), device->GetRawState() });
		} else {
			states.push_back({ ControllerType::None, ControlDeviceState()});
		}
	}
	return states;
}

SystemActionManager* ControlManager::GetSystemActionManager()
{
	return _systemActionManager.get();
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
	return _emu->GetSettings()->GetInputConfig().Controllers[port].Type;
}

shared_ptr<BaseControlDevice> ControlManager::CreateControllerDevice(ControllerType type, uint8_t port, Console* console)
{
	shared_ptr<BaseControlDevice> device;
	
	InputConfig cfg = console->GetEmulator()->GetSettings()->GetInputConfig();

	switch(type) {
		case ControllerType::None: break;
		case ControllerType::SnesController: device.reset(new SnesController(console, port, cfg.Controllers[port].Keys)); break;
		case ControllerType::SnesMouse: device.reset(new SnesMouse(console, port)); break;
		case ControllerType::SuperScope: device.reset(new SuperScope(console, port, cfg.Controllers[port].Keys)); break;
		case ControllerType::Multitap: device.reset(new Multitap(console, port, cfg.Controllers[port].Keys, cfg.Controllers[2].Keys, cfg.Controllers[3].Keys, cfg.Controllers[4].Keys)); break;
	}
	
	return device;
}

void ControlManager::UpdateControlDevices()
{
	uint32_t version = _emu->GetSettings()->GetInputConfigVersion();
	if(_inputConfigVersion != version) {
		_inputConfigVersion = version;

		auto lock = _deviceLock.AcquireSafe();
		_controlDevices.clear();
		RegisterControlDevice(_systemActionManager);
		for(int i = 0; i < 2; i++) {
			shared_ptr<BaseControlDevice> device = CreateControllerDevice(GetControllerType(i), i, _console);
			if(device) {
				RegisterControlDevice(device);
			}
		}
	}
}

void ControlManager::UpdateInputState()
{
	KeyManager::RefreshKeyState();

	auto lock = _deviceLock.AcquireSafe();

	//string log = "F: " + std::to_string(_console->GetPpu()->GetFrameCount()) + " C:" + std::to_string(_pollCounter) + " ";
	for(shared_ptr<BaseControlDevice> &device : _controlDevices) {
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
	InputConfig cfg = _emu->GetSettings()->GetInputConfig();
	s.Stream(cfg.Controllers[0].Type, cfg.Controllers[1].Type, cfg.Controllers[2].Type, cfg.Controllers[3].Type, cfg.Controllers[4].Type);
	if(!s.IsSaving()) {
		_emu->GetSettings()->SetInputConfig(cfg);
		UpdateControlDevices();
	}

	for(shared_ptr<BaseControlDevice> &device : _controlDevices) {
		s.Stream(device.get());
	}
}
