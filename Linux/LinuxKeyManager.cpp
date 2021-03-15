#include <algorithm>
#include "LinuxKeyManager.h"
#include "LinuxGameController.h"
#include "../Utilities/FolderUtilities.h"
#include "../Core/ControlManager.h"
#include "../Core/Console.h"

static vector<KeyDefinition> _keyDefinitions = {
	{ "None", 0 },
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

LinuxKeyManager::LinuxKeyManager(shared_ptr<Console> console)
{
	_console = console;

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
				std::shared_ptr<LinuxGameController> controller = LinuxGameController::GetController(_console, deviceId, logInformation);
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
				_console->Pause();
				for(int index : indexesToRemove) {
					_controllers.erase(_controllers.begin()+index);
				}
				for(std::shared_ptr<LinuxGameController> controller : controllersToAdd) {
					_controllers.push_back(controller);
				} 
				_console->Resume();
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
