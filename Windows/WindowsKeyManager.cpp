#include "stdafx.h"
#include "WindowsKeyManager.h"
#include "Shared/KeyDefinitions.h"

WindowsKeyManager::WindowsKeyManager(shared_ptr<Emulator> emu, HWND hWnd)
{
	_emu = emu;
	_hWnd = hWnd;

	ResetKeyState();

	//Init XInput buttons
	vector<string> buttonNames = { "Up", "Down", "Left", "Right", "Start", "Back", "L3", "R3", "L1", "R1", "?", "?", "A", "B", "X", "Y", "L2", "R2", "RT Up", "RT Down", "RT Left", "RT Right", "LT Up", "LT Down", "LT Left", "LT Right" };
	for(int i = 0; i < 4; i++) {
		for(int j = 0; j < (int)buttonNames.size(); j++) {
			_keyDefinitions.push_back({ "Pad" + std::to_string(i + 1) + " " + buttonNames[j], (uint32_t)(0xFFFF + i * 0x100 + j + 1) });
		}
	}

	//Init DirectInput buttons
	vector<string> diButtonNames = { "Y+", "Y-", "X-", "X+", "Y2+", "Y2-", "X2-", "X2+", "Z+", "Z-", "Z2+", "Z2-", "DPad Up", "DPad Down", "DPad Right", "DPad Left" };
	for(int i = 0; i < 16; i++) {
		for(int j = 0; j < (int)diButtonNames.size(); j++) {
			_keyDefinitions.push_back({ "Joy" + std::to_string(i + 1) + " " + diButtonNames[j], (uint32_t)(0x11000 + i * 0x100 + j) });
		}

		for(int j = 0; j < 128; j++) {
			_keyDefinitions.push_back({ "Joy" + std::to_string(i + 1) + " But" + std::to_string(j + 1), (uint32_t)(0x11000 + i * 0x100 + j + 0x10)});
		}
	}

	for(KeyDefinition& keyDef : _keyDefinitions) {
		_keyNames[keyDef.keyCode] = keyDef.name;
		_keyCodes[keyDef.name] = keyDef.keyCode;
	}
	
	StartUpdateDeviceThread();
}

WindowsKeyManager::~WindowsKeyManager()
{
	_stopUpdateDeviceThread = true;
	_stopSignal.Signal();
	_updateDeviceThread.join();
}

void WindowsKeyManager::StartUpdateDeviceThread()
{
	_updateDeviceThread = std::thread([=]() {
		_xInput.reset(new XInputManager(_emu));
		_directInput.reset(new DirectInputManager(_emu, _hWnd));

		while(!_stopUpdateDeviceThread) {
			//Check for newly plugged in XInput controllers every 5 secs
			//Do not check for DirectInput controllers because this takes more time and sometimes causes issues/freezes
			if(_xInput->NeedToUpdate()) {
				_xInput->UpdateDeviceList();
			}
			_stopSignal.Wait(5000);
		}
	});
}

void WindowsKeyManager::RefreshState()
{
	if(!_xInput || !_directInput) {
		return;
	}

	_xInput->RefreshState();
	_directInput->RefreshState();
}

bool WindowsKeyManager::IsKeyPressed(uint32_t key)
{
	if(_disableAllKeys) {
		return false;
	}

	if(key >= 0x10000) {
		if(!_xInput || !_directInput) {
			return false;
		}

		if(key >= 0x11000) {
			//Directinput key
			uint8_t gamepadPort = (key - 0x11000) / 0x100;
			uint8_t gamepadButton = (key - 0x11000) % 0x100;
			return _directInput->IsPressed(gamepadPort, gamepadButton);
		} else {
			//XInput key
			uint8_t gamepadPort = (key - 0xFFFF) / 0x100;
			uint8_t gamepadButton = (key - 0xFFFF) % 0x100;
			return _xInput->IsPressed(gamepadPort, gamepadButton);
		}
	} else if(key < 0x200) {
		return _keyState[key] != 0;
	}
	return false;
}

bool WindowsKeyManager::IsMouseButtonPressed(MouseButton button)
{
	switch(button) {
		case MouseButton::LeftButton: return _mouseState[0];
		case MouseButton::RightButton: return _mouseState[1];
		case MouseButton::MiddleButton: return _mouseState[2];
	}

	return false;
}

vector<uint32_t> WindowsKeyManager::GetPressedKeys()
{
	vector<uint32_t> result;
	if(!_xInput || !_directInput) {
		return result;
	}

	_xInput->RefreshState();
	for(int i = 0; i < XUSER_MAX_COUNT; i++) {
		for(int j = 1; j <= 26; j++) {
			if(_xInput->IsPressed(i, j)) {
				result.push_back(0xFFFF + i * 0x100 + j);
			}
		}
	}

	_directInput->RefreshState();
	for(int i = _directInput->GetJoystickCount() - 1; i >= 0; i--) {
		for(int j = 0; j < 16+128; j++) {
			if(_directInput->IsPressed(i, j)) {
				result.push_back(0x11000 + i * 0x100 + j);
			}
		}
	}

	for(int i = 0; i < 0x200; i++) {
		if(_keyState[i]) {
			result.push_back(i);
		}
	}
	return result;
}

string WindowsKeyManager::GetKeyName(uint32_t keyCode)
{
	auto keyDef = _keyNames.find(keyCode);
	if(keyDef != _keyNames.end()) {
		return keyDef->second;
	}
	return "";
}

uint32_t WindowsKeyManager::GetKeyCode(string keyName)
{
	auto keyDef = _keyCodes.find(keyName);
	if(keyDef != _keyCodes.end()) {
		return keyDef->second;
	}
	return 0;
}

void WindowsKeyManager::UpdateDevices()
{
	if(!_xInput || !_directInput) {
		return;
	}

	_xInput->UpdateDeviceList();
	_directInput->UpdateDeviceList();
}

void WindowsKeyManager::SetKeyState(uint16_t scanCode, bool state)
{
	if(scanCode > 0xFFF) {
		_mouseState[scanCode & 0x03] = state;
	} else {
		_keyState[scanCode] = state;
	}
}

void WindowsKeyManager::SetDisabled(bool disabled)
{
	_disableAllKeys = disabled;
}

void WindowsKeyManager::ResetKeyState()
{
	memset(_mouseState, 0, sizeof(_mouseState));
	memset(_keyState, 0, sizeof(_keyState));
}