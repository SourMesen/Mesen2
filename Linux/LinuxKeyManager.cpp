#include <algorithm>
#include "LinuxKeyManager.h"
#include "LinuxGameController.h"
#include "../Utilities/FolderUtilities.h"
#include "../Core/ControlManager.h"
#include "../Core/Console.h"

static vector<KeyDefinition> _keyDefinitions = {
	{ "", 9, "Esc", "" },
	{ "", 10, "1", "" },
	{ "", 11, "2", "" },
	{ "", 12, "3", "" },
	{ "", 13, "4", "" },
	{ "", 14, "5", "" },
	{ "", 15, "6", "" },
	{ "", 16, "7", "" },
	{ "", 17, "8", "" },
	{ "", 18, "9", "" },
	{ "", 19, "0", "" },
	{ "", 20, "-", "" },
	{ "", 21, "=", "" },
	{ "", 22, "Backspace", "" },
	{ "", 23, "Tab", "" },
	{ "", 24, "Q", "" },
	{ "", 25, "W", "" },
	{ "", 26, "E", "" },
	{ "", 27, "R", "" },
	{ "", 28, "T", "" },
	{ "", 29, "Y", "" },
	{ "", 30, "U", "" },
	{ "", 31, "I", "" },
	{ "", 32, "O", "" },
	{ "", 33, "P", "" },
	{ "", 34, "[", "" },
	{ "", 35, "]", "" },
	{ "", 36, "Enter", "" },
	{ "", 37, "Ctrl", "" },
	{ "", 38, "A", "" },
	{ "", 39, "S", "" },
	{ "", 40, "D", "" },
	{ "", 41, "F", "" },
	{ "", 42, "G", "" },
	{ "", 43, "H", "" },
	{ "", 44, "J", "" },
	{ "", 45, "K", "" },
	{ "", 46, "L", "" },
	{ "", 47, ";", "" },
	{ "", 48, "'", "" },
	{ "", 49, "`", "" },
	{ "", 50, "Shift", "" },
	{ "", 51, "\\", "" },
	{ "", 52, "Z", "" },
	{ "", 53, "X", "" },
	{ "", 54, "C", "" },
	{ "", 55, "V", "" },
	{ "", 56, "B", "" },
	{ "", 57, "N", "" },
	{ "", 58, "M", "" },
	{ "", 59, ",", "" },
	{ "", 60, ".", "" },
	{ "", 61, "/", "" },
	{ "", 62, "Right Shift", "" },
	{ "", 63, "Numpad *", "" },
	{ "", 64, "Alt", "" },
	{ "", 65, "Spacebar", "" },
	{ "", 66, "Caps Lock", "" },
	{ "", 67, "F1", "" },
	{ "", 68, "F2", "" },
	{ "", 69, "F3", "" },
	{ "", 70, "F4", "" },
	{ "", 71, "F5", "" },
	{ "", 72, "F6", "" },
	{ "", 73, "F7", "" },
	{ "", 74, "F8", "" },
	{ "", 75, "F9", "" },
	{ "", 76, "F10", "" },
	{ "", 77, "Num Lock", "" },
	{ "", 78, "Scroll Lock", "" },
	{ "", 79, "Numpad 7", "" },
	{ "", 80, "Numpad 8", "" },
	{ "", 81, "Numpad 9", "" },
	{ "", 82, "Numpad -", "" },
	{ "", 83, "Numpad 4", "" },
	{ "", 84, "Numpad 5", "" },
	{ "", 85, "Numpad 6", "" },
	{ "", 86, "Numpad +", "" },
	{ "", 87, "Numpad 1", "" },
	{ "", 88, "Numpad 2", "" },
	{ "", 89, "Numpad 3", "" },
	{ "", 90, "Numpad 0", "" },
	{ "", 91, "Numpad .", "" },
	{ "", 92, "ISO_Level3_Shift", "" },
	{ "", 94, "Pipe", "" },
	{ "", 95, "F11", "" },
	{ "", 96, "F12", "" },
	{ "", 98, "Katakana", "" },
	{ "", 99, "Hiragana", "" },
	{ "", 100, "Henkan_Mode", "" },
	{ "", 101, "Hiragana_Katakana", "" },
	{ "", 102, "Muhenkan", "" },
	{ "", 104, "Numpad Enter", "" },
	{ "", 105, "Control_R", "" },
	{ "", 106, "Numpad /", "" },
	{ "", 107, "Print", "" },
	{ "", 108, "Right Alt", "" },
	{ "", 109, "Linefeed", "" },
	{ "", 110, "Home", "" },
	{ "", 111, "Up Arrow", "" },
	{ "", 112, "Page Up", "" },
	{ "", 113, "Left Arrow", "" },
	{ "", 114, "Right Arrow", "" },
	{ "", 115, "End", "" },
	{ "", 116, "Down Arrow", "" },
	{ "", 117, "Page Down", "" },
	{ "", 118, "Insert", "" },
	{ "", 119, "Delete", "" },
	{ "", 121, "XF86AudioMute", "" },
	{ "", 122, "XF86AudioLowerVolume", "" },
	{ "", 123, "XF86AudioRaiseVolume", "" },
	{ "", 124, "XF86PowerOff", "" },
	{ "", 125, "KP_Equal", "" },
	{ "", 126, "PlusMinus", "" },
	{ "", 127, "Pause", "" },
	{ "", 128, "XF86LaunchA", "" },
	{ "", 129, "KP_Decimal", "" },
	{ "", 130, "Hangul", "" },
	{ "", 131, "Hangul_Hanja", "" },
	{ "", 133, "Super_L", "" },
	{ "", 134, "Super_R", "" },
	{ "", 135, "Menu", "" },
	{ "", 136, "Cancel", "" },
	{ "", 137, "Redo", "" },
	{ "", 138, "SunProps", "" },
	{ "", 139, "Undo", "" },
	{ "", 140, "SunFront", "" },
	{ "", 141, "XF86Copy", "" },
	{ "", 142, "XF86Open", "" },
	{ "", 143, "XF86Paste", "" },
	{ "", 144, "Find", "" },
	{ "", 145, "XF86Cut", "" },
	{ "", 146, "Help", "" },
	{ "", 147, "XF86MenuKB", "" },
	{ "", 148, "XF86Calculator", "" },
	{ "", 150, "XF86Sleep", "" },
	{ "", 151, "XF86WakeUp", "" },
	{ "", 152, "XF86Explorer", "" },
	{ "", 153, "XF86Send", "" },
	{ "", 155, "XF86Xfer", "" },
	{ "", 156, "XF86Launch1", "" },
	{ "", 157, "XF86Launch2", "" },
	{ "", 158, "XF86WWW", "" },
	{ "", 159, "XF86DOS", "" },
	{ "", 160, "XF86ScreenSaver", "" },
	{ "", 161, "XF86RotateWindows", "" },
	{ "", 162, "XF86TaskPane", "" },
	{ "", 163, "XF86Mail", "" },
	{ "", 164, "XF86Favorites", "" },
	{ "", 165, "XF86MyComputer", "" },
	{ "", 166, "XF86Back", "" },
	{ "", 167, "XF86Forward", "" },
	{ "", 169, "XF86Eject", "" },
	{ "", 170, "XF86Eject", "" },
	{ "", 171, "XF86AudioNext", "" },
	{ "", 172, "XF86AudioPlay", "" },
	{ "", 173, "XF86AudioPrev", "" },
	{ "", 174, "XF86AudioStop", "" },
	{ "", 175, "XF86AudioRecord", "" },
	{ "", 176, "XF86AudioRewind", "" },
	{ "", 177, "XF86Phone", "" },
	{ "", 179, "XF86Tools", "" },
	{ "", 180, "XF86HomePage", "" },
	{ "", 181, "XF86Reload", "" },
	{ "", 182, "XF86Close", "" },
	{ "", 185, "XF86ScrollUp", "" },
	{ "", 186, "XF86ScrollDown", "" },
	{ "", 187, "Paren Left", "" },
	{ "", 188, "Paren Right", "" },
	{ "", 189, "XF86New", "" },
	{ "", 190, "Redo", "" },
	{ "", 191, "XF86Tools", "" },
	{ "", 192, "XF86Launch5", "" },
	{ "", 193, "XF86Launch6", "" },
	{ "", 194, "XF86Launch7", "" },
	{ "", 195, "XF86Launch8", "" },
	{ "", 196, "XF86Launch9", "" },
	{ "", 198, "XF86AudioMicMute", "" },
	{ "", 199, "XF86TouchpadToggle", "" },
	{ "", 200, "XF86TouchpadOn", "" },
	{ "", 201, "XF86TouchpadOff", "" },
	{ "", 203, "Mode_switch", "" },
	{ "", 204, "NoSymbol", "" },
	{ "", 205, "NoSymbol", "" },
	{ "", 206, "NoSymbol", "" },
	{ "", 207, "NoSymbol", "" },
	{ "", 208, "XF86AudioPlay", "" },
	{ "", 209, "XF86AudioPause", "" },
	{ "", 210, "XF86Launch3", "" },
	{ "", 211, "XF86Launch4", "" },
	{ "", 212, "XF86LaunchB", "" },
	{ "", 213, "XF86Suspend", "" },
	{ "", 214, "XF86Close", "" },
	{ "", 215, "XF86AudioPlay", "" },
	{ "", 216, "XF86AudioForward", "" },
	{ "", 218, "Print", "" },
	{ "", 220, "XF86WebCam", "" },
	{ "", 223, "XF86Mail", "" },
	{ "", 224, "XF86Messenger", "" },
	{ "", 225, "XF86Search", "" },
	{ "", 226, "XF86Go", "" },
	{ "", 227, "XF86Finance", "" },
	{ "", 228, "XF86Game", "" },
	{ "", 229, "XF86Shop", "" },
	{ "", 231, "Cancel", "" },
	{ "", 232, "XF86MonBrightnessDown", "" },
	{ "", 233, "XF86MonBrightnessUp", "" },
	{ "", 234, "XF86AudioMedia", "" },
	{ "", 235, "XF86Display", "" },
	{ "", 236, "XF86KbdLightOnOff", "" },
	{ "", 237, "XF86KbdBrightnessDown", "" },
	{ "", 238, "XF86KbdBrightnessUp", "" },
	{ "", 239, "XF86Send", "" },
	{ "", 240, "XF86Reply", "" },
	{ "", 241, "XF86MailForward", "" },
	{ "", 242, "XF86Save", "" },
	{ "", 243, "XF86Documents", "" },
	{ "", 244, "XF86Battery", "" },
	{ "", 245, "XF86Bluetooth", "" },
	{ "", 246, "XF86WLAN", "" },
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
			_keyDefinitions.push_back({ "", (uint32_t)(0x10000 + i * 0x100 + j), "Pad" + std::to_string(i + 1) + " " + buttonNames[j], "" });
		}
	}

	for(KeyDefinition &keyDef : _keyDefinitions) {
		_keyNames[keyDef.keyCode] = keyDef.description;
		_keyCodes[keyDef.description] = keyDef.keyCode;
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
	} else if(key < 0x200) {
		return _keyState[key & 0xFF] != 0;
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
	if(scanCode > 0x1FF) {
		_mouseState[scanCode & 0x03] = state;
	} else {
		scanCode &= 0xFF;
		if(scanCode == 105) {
			//Left + Right Ctrl
			scanCode = 37;
		} else if(scanCode == 62) {
			//Left + Right Shift
			scanCode = 50;
		} else if(scanCode == 108) {
			//Left + Right Alt
			scanCode = 64;
		}
		_keyState[scanCode & 0xFF] = state;
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
