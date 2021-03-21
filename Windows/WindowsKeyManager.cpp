#include "stdafx.h"
#include "WindowsKeyManager.h"

static vector<KeyDefinition> _keyDefinitions = {
	{ "", 0 },
	{ "Cancel", 1 },
	{ "Back", 2 },
	{ "Tab", 3 },
	{ "LineFeed", 4 },
	{ "Clear", 5 },
	{ "Return", 6 },
	{ "Enter", 6 },
	{ "Pause", 7 },
	{ "CapsLock", 8 },
	{ "Capital", 8 },
	{ "HangulMode", 9 },
	{ "KanaMode", 9 },
	{ "JunjaMode", 10 },
	{ "FinalMode", 11 },
	{ "KanjiMode", 12 },
	{ "HanjaMode", 12 },
	{ "Escape", 13 },
	{ "ImeConvert", 14 },
	{ "ImeNonConvert", 0xF },
	{ "ImeAccept", 0x10 },
	{ "ImeModeChange", 17 },
	{ "Space", 18 },
	{ "PageUp", 19 },
	{ "Prior", 19 },
	{ "PageDown", 20 },
	{ "Next", 20 },
	{ "End", 21 },
	{ "Home", 22 },
	{ "Left", 23 },
	{ "Up", 24 },
	{ "Right", 25 },
	{ "Down", 26 },
	{ "Select", 27 },
	{ "Print", 28 },
	{ "Execute", 29 },
	{ "Snapshot", 30 },
	{ "PrintScreen", 30 },
	{ "Insert", 0x1F },
	{ "Delete", 0x20 },
	{ "Help", 33 },
	{ "D0", 34 },
	{ "D1", 35 },
	{ "D2", 36 },
	{ "D3", 37 },
	{ "D4", 38 },
	{ "D5", 39 },
	{ "D6", 40 },
	{ "D7", 41 },
	{ "D8", 42 },
	{ "D9", 43 },
	{ "A", 44 },
	{ "B", 45 },
	{ "C", 46 },
	{ "D", 47 },
	{ "E", 48 },
	{ "F", 49 },
	{ "G", 50 },
	{ "H", 51 },
	{ "I", 52 },
	{ "J", 53 },
	{ "K", 54 },
	{ "L", 55 },
	{ "M", 56 },
	{ "N", 57 },
	{ "O", 58 },
	{ "P", 59 },
	{ "Q", 60 },
	{ "R", 61 },
	{ "S", 62 },
	{ "T", 0x3F },
	{ "U", 0x40 },
	{ "V", 65 },
	{ "W", 66 },
	{ "X", 67 },
	{ "Y", 68 },
	{ "Z", 69 },
	{ "LWin", 70 },
	{ "RWin", 71 },
	{ "Apps", 72 },
	{ "Sleep", 73 },
	{ "NumPad0", 74 },
	{ "NumPad1", 75 },
	{ "NumPad2", 76 },
	{ "NumPad3", 77 },
	{ "NumPad4", 78 },
	{ "NumPad5", 79 },
	{ "NumPad6", 80 },
	{ "NumPad7", 81 },
	{ "NumPad8", 82 },
	{ "NumPad9", 83 },
	{ "Multiply", 84 },
	{ "Add", 85 },
	{ "Separator", 86 },
	{ "Subtract", 87 },
	{ "Decimal", 88 },
	{ "Divide", 89 },
	{ "F1", 90 },
	{ "F2", 91 },
	{ "F3", 92 },
	{ "F4", 93 },
	{ "F5", 94 },
	{ "F6", 95 },
	{ "F7", 96 },
	{ "F8", 97 },
	{ "F9", 98 },
	{ "F10", 99 },
	{ "F11", 100 },
	{ "F12", 101 },
	{ "F13", 102 },
	{ "F14", 103 },
	{ "F15", 104 },
	{ "F16", 105 },
	{ "F17", 106 },
	{ "F18", 107 },
	{ "F19", 108 },
	{ "F20", 109 },
	{ "F21", 110 },
	{ "F22", 111 },
	{ "F23", 112 },
	{ "F24", 113 },
	{ "NumLock", 114 },
	{ "Scroll", 115 },
	{ "LeftShift", 116 },
	{ "RightShift", 117 },
	{ "LeftCtrl", 118 },
	{ "RightCtrl", 119 },
	{ "LeftAlt", 120 },
	{ "RightAlt", 121 },
	{ "BrowserBack", 122 },
	{ "BrowserForward", 123 },
	{ "BrowserRefresh", 124 },
	{ "BrowserStop", 125 },
	{ "BrowserSearch", 126 },
	{ "BrowserFavorites", 0x7F },
	{ "BrowserHome", 0x80 },
	{ "VolumeMute", 129 },
	{ "VolumeDown", 130 },
	{ "VolumeUp", 131 },
	{ "MediaNextTrack", 132 },
	{ "MediaPreviousTrack", 133 },
	{ "MediaStop", 134 },
	{ "MediaPlayPause", 135 },
	{ "LaunchMail", 136 },
	{ "SelectMedia", 137 },
	{ "LaunchApplication1", 138 },
	{ "LaunchApplication2", 139 },
	{ "OemSemicolon", 140 },
	{ "Oem1", 140 },
	{ "OemPlus", 141 },
	{ "OemComma", 142 },
	{ "OemMinus", 143 },
	{ "OemPeriod", 144 },
	{ "OemQuestion", 145 },
	{ "Oem2", 145 },
	{ "OemTilde", 146 },
	{ "Oem3", 146 },
	{ "AbntC1", 147 },
	{ "AbntC2", 148 },
	{ "OemOpenBrackets", 149 },
	{ "Oem4", 149 },
	{ "OemPipe", 150 },
	{ "Oem5", 150 },
	{ "OemCloseBrackets", 151 },
	{ "Oem6", 151 },
	{ "OemQuotes", 152 },
	{ "Oem7", 152 },
	{ "Oem8", 153 },
	{ "OemBackslash", 154 },
	{ "Oem102", 154 },
	{ "ImeProcessed", 155 },
	{ "System", 156 },
	{ "OemAttn", 157 },
	{ "DbeAlphanumeric", 157 },
	{ "OemFinish", 158 },
	{ "DbeKatakana", 158 },
	{ "DbeHiragana", 159 },
	{ "OemCopy", 159 },
	{ "DbeSbcsChar", 160 },
	{ "OemAuto", 160 },
	{ "DbeDbcsChar", 161 },
	{ "OemEnlw", 161 },
	{ "OemBackTab", 162 },
	{ "DbeRoman", 162 },
	{ "DbeNoRoman", 163 },
	{ "Attn", 163 },
	{ "CrSel", 164 },
	{ "DbeEnterWordRegisterMode", 164 },
	{ "ExSel", 165 },
	{ "DbeEnterImeConfigureMode", 165 },
	{ "EraseEof", 166 },
	{ "DbeFlushString", 166 },
	{ "Play", 167 },
	{ "DbeCodeInput", 167 },
	{ "DbeNoCodeInput", 168 },
	{ "Zoom", 168 },
	{ "NoName", 169 },
	{ "DbeDetermineString", 169 },
	{ "DbeEnterDialogConversionMode", 170 },
	{ "Pa1", 170 },
	{ "OemClear", 171 },
	{ "DeadCharProcessed", 172 },
	{ "FnLeftArrow", 10001 },
	{ "FnRightArrow", 10002 },
	{ "FnUpArrow", 10003 },
	{ "FnDownArrow", 10004 }
};

WindowsKeyManager::WindowsKeyManager(shared_ptr<Console> console, HWND hWnd)
{
	_console = console;
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
		_xInput.reset(new XInputManager(_console));
		_directInput.reset(new DirectInputManager(_console, _hWnd));

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