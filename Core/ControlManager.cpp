#include "stdafx.h"
#include "ControlManager.h"
#include "Console.h"
#include "MemoryManager.h"
#include "KeyManager.h"
#include "IKeyManager.h"
#include "IInputProvider.h"
#include "IInputRecorder.h"
#include "SnesController.h"

ControlManager::ControlManager(shared_ptr<Console> console)
{
	_console = console;
	_pollCounter = 0;

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
	return ControllerType::SnesController;
	//TODO _console->GetSettings()->GetControllerType(port);
}

shared_ptr<BaseControlDevice> ControlManager::CreateControllerDevice(ControllerType type, uint8_t port, shared_ptr<Console> console)
{
	shared_ptr<BaseControlDevice> device;

	KeyMappingSet keyMappings = {};
	if(port == 0) {
		keyMappings.Mapping1.Up = KeyManager::GetKeyCode("Pad1 Up");
		keyMappings.Mapping1.Down = KeyManager::GetKeyCode("Pad1 Down");
		keyMappings.Mapping1.Left = KeyManager::GetKeyCode("Pad1 Left");
		keyMappings.Mapping1.Right = KeyManager::GetKeyCode("Pad1 Right");
		keyMappings.Mapping1.A = KeyManager::GetKeyCode("Pad1 B");
		keyMappings.Mapping1.B = KeyManager::GetKeyCode("Pad1 A");
		keyMappings.Mapping1.X = KeyManager::GetKeyCode("Pad1 Y");
		keyMappings.Mapping1.Y = KeyManager::GetKeyCode("Pad1 X");
		keyMappings.Mapping1.L = KeyManager::GetKeyCode("Pad1 L1");
		keyMappings.Mapping1.R = KeyManager::GetKeyCode("Pad1 R1");
		keyMappings.Mapping1.Select = KeyManager::GetKeyCode("Pad1 Back");
		keyMappings.Mapping1.Start = KeyManager::GetKeyCode("Pad1 Start");

		keyMappings.Mapping2.Up = KeyManager::GetKeyCode("Up Arrow");
		keyMappings.Mapping2.Down = KeyManager::GetKeyCode("Down Arrow");
		keyMappings.Mapping2.Left = KeyManager::GetKeyCode("Left Arrow");
		keyMappings.Mapping2.Right = KeyManager::GetKeyCode("Right Arrow");
		keyMappings.Mapping2.A = KeyManager::GetKeyCode("Z");
		keyMappings.Mapping2.B = KeyManager::GetKeyCode("X");
		keyMappings.Mapping2.X = KeyManager::GetKeyCode("S");
		keyMappings.Mapping2.Y = KeyManager::GetKeyCode("A");
		keyMappings.Mapping2.L = KeyManager::GetKeyCode("Q");
		keyMappings.Mapping2.R = KeyManager::GetKeyCode("W");
		keyMappings.Mapping2.Select = KeyManager::GetKeyCode("E");
		keyMappings.Mapping2.Start = KeyManager::GetKeyCode("D");
	}

	switch(type) {
		case ControllerType::None: break;
		case ControllerType::SnesController: device.reset(new SnesController(console, port, keyMappings)); break;
	}
	
	return device;
}

void ControlManager::UpdateControlDevices()
{
	auto lock = _deviceLock.AcquireSafe();
	_controlDevices.clear();

	for(int i = 0; i < 4; i++) {
		shared_ptr<BaseControlDevice> device = CreateControllerDevice(GetControllerType(i), i, _console);
		if(device) {
			RegisterControlDevice(device);
		}
	}
}

uint8_t ControlManager::GetOpenBusMask(uint8_t port)
{
	return 0;
}

void ControlManager::UpdateInputState()
{
	KeyManager::RefreshKeyState();

	//auto lock = _deviceLock.AcquireSafe();

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
	uint8_t value = 0; //TODO _console->GetMemoryManager()->GetOpenBus(GetOpenBusMask(addr - 0x4016));
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