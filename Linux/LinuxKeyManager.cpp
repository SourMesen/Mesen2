#include <algorithm>
#include "LinuxKeyManager.h"
#include "LinuxGameController.h"
#include "Utilities/FolderUtilities.h"
#include "Shared/Emulator.h"
#include "Shared/KeyDefinitions.h"

LinuxKeyManager::LinuxKeyManager(Emulator* emu)
{
	_emu = emu;

	ResetKeyState();

	vector<string> buttonNames = { 
		"A", "B", "C", "X", "Y", "Z", "L1", "R1", "L2", "R2", "Select", "Start", "L3", "R3", 
		"X+", "X-", "Y+", "Y-", "Z+", "Z-", 
		"X2+", "X2-", "Y2+", "Y2-", "Z2+", "Z2-", 
		"Right", "Left", "Down", "Up", 
		"Right 2", "Left 2", "Down 2", "Up 2", 
		"Right 3", "Left 3", "Down 3", "Up 3", 
		"Right 4", "Left 4", "Down 4", "Up 4",
		"Trigger", "Thumb", "Thumb2", "Top", "Top2",
		"Pinkie", "Base", "Base2", "Base3", "Base4",
		"Base5", "Base6", "Dead"
	};

	for(int i = 0; i < 20; i++) {
		for(int j = 0; j < (int)buttonNames.size(); j++) {
			_keyDefinitions.push_back({ "Pad" + std::to_string(i + 1) + " " + buttonNames[j], (uint32_t)(0x10000 + i * 0x100 + j) });
		}
	}

	for(KeyDefinition &keyDef : _keyDefinitions) {
		_keyNames[keyDef.keyCode] = keyDef.name;
		_keyCodes[keyDef.name] = keyDef.keyCode;
	}

	CheckForGamepads(true);

	_disableAllKeys = false;
	_stopUpdateDeviceThread = false;
	StartUpdateDeviceThread();	
}

LinuxKeyManager::~LinuxKeyManager()
{
	_stopUpdateDeviceThread = true;
	_stopSignal.Signal();
	_updateDeviceThread.join();
}

void LinuxKeyManager::RefreshState()
{
	//TODO: NOT IMPLEMENTED YET;
	//Only needed to detect poll controller input
}

bool LinuxKeyManager::IsKeyPressed(uint32_t key)
{
	if(_disableAllKeys) {
		return false;
	}

	if(key >= 0x10000) {
		uint8_t gamepadPort = (key - 0x10000) / 0x100;
		uint8_t gamepadButton = (key - 0x10000) % 0x100;
		if(_controllers.size() > gamepadPort) {
			return _controllers[gamepadPort]->IsButtonPressed(gamepadButton);
		}
	} else if(key) {
		return _keyState[key] != 0;
	}
	return false;
}

bool LinuxKeyManager::IsMouseButtonPressed(MouseButton button)
{
	switch(button) {
		case MouseButton::LeftButton: return _mouseState[0];
		case MouseButton::RightButton: return _mouseState[1];
		case MouseButton::MiddleButton: return _mouseState[2];
	}

	return false;
}

vector<uint32_t> LinuxKeyManager::GetPressedKeys()
{
	vector<uint32_t> pressedKeys;
	for(size_t i = 0; i < _controllers.size(); i++) {
		for(int j = 0; j <= 54; j++) {
			if(_controllers[i]->IsButtonPressed(j)) {
				pressedKeys.push_back(0x10000 + i * 0x100 + j);
			}
		}
	}

	for(int i = 0; i < 0x200; i++) {
		if(_keyState[i]) {
			pressedKeys.push_back(i);
		}
	}
	return pressedKeys;
}

string LinuxKeyManager::GetKeyName(uint32_t key)
{
	auto keyDef = _keyNames.find(key);
	if(keyDef != _keyNames.end()) {
		return keyDef->second;
	}
	return "";
}

uint32_t LinuxKeyManager::GetKeyCode(string keyName)
{
	auto keyDef = _keyCodes.find(keyName);
	if(keyDef != _keyCodes.end()) {
		return keyDef->second;
	}
	return 0;
}

void LinuxKeyManager::UpdateDevices()
{
	//TODO: NOT IMPLEMENTED YET
	//Only needed to detect newly plugged in devices
}

void LinuxKeyManager::CheckForGamepads(bool logInformation)
{
	vector<int> connectedIDs; 
	for(int i = _controllers.size() - 1; i >= 0; i--) {
		if(!_controllers[i]->IsDisconnected()) {
			connectedIDs.push_back(_controllers[i]->GetDeviceID());
		}
	}

	vector<string> files = FolderUtilities::GetFilesInFolder("/dev/input/", {}, false);
	for(size_t i = 0; i < files.size(); i++) {
		string filename = FolderUtilities::GetFilename(files[i], false);
		if(filename.find("event", 0) == 0) {
			int deviceId = 0;
			try {
				deviceId = std::stoi(filename.substr(5));
			} catch(std::exception e) {
				continue;
			}

			if(std::find(connectedIDs.begin(), connectedIDs.end(), deviceId) == connectedIDs.end()) {
				std::shared_ptr<LinuxGameController> controller = LinuxGameController::GetController(_emu, deviceId, logInformation);
				if(controller) {
					_controllers.push_back(controller);
				}
			}
		}
	}
}

void LinuxKeyManager::StartUpdateDeviceThread()
{
	_updateDeviceThread = std::thread([=]() {
		while(!_stopUpdateDeviceThread) {
			//Check for newly plugged in controllers every 5 secs
			vector<shared_ptr<LinuxGameController>> controllersToAdd; 
			vector<int> indexesToRemove;
			for(int i = _controllers.size() - 1; i >= 0; i--) {
				if(_controllers[i]->IsDisconnected()) {
					indexesToRemove.push_back(i);
				}
			}

			CheckForGamepads(false);

			if(!indexesToRemove.empty() || !controllersToAdd.empty()) {
				_emu->Pause();
				for(int index : indexesToRemove) {
					_controllers.erase(_controllers.begin()+index);
				}
				for(std::shared_ptr<LinuxGameController> controller : controllersToAdd) {
					_controllers.push_back(controller);
				} 
				_emu->Resume();
			}

			_stopSignal.Wait(5000);
		}
	});
}	

void LinuxKeyManager::SetKeyState(uint16_t scanCode, bool state)
{
	if(scanCode > 0xFFF) {
		_mouseState[scanCode & 0x03] = state;
	} else {
		_keyState[scanCode] = state;
	}
}

void LinuxKeyManager::ResetKeyState()
{
	memset(_mouseState, 0, sizeof(_mouseState));
	memset(_keyState, 0, sizeof(_keyState));
}

void LinuxKeyManager::SetDisabled(bool disabled)
{
	_disableAllKeys = disabled;
}
