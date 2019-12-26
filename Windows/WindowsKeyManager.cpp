#include "stdafx.h"
#include "WindowsKeyManager.h"

static vector<KeyDefinition> _keyDefinitions = {
	//{ "VK_LBUTTON", 0x01, "Left mouse button", "" },
	//{ "VK_RBUTTON", 0x02, "Right mouse button", "" },
	{ "VK_CANCEL", 0x03, "Control-break processing", "" },
	//{ "VK_MBUTTON", 0x04, "Middle mouse button (three-button mouse)", "" },
	//{ "VK_XBUTTON1", 0x05, "X1 mouse button", "" },
	//{ "VK_XBUTTON2", 0x06, "X2 mouse button", "" },
	{ "-", 0x07, "Undefined", "" },
	{ "VK_BACK", 0x08, "Backspace", "" },
	{ "VK_TAB", 0x09, "Tab", "" },
	//{ "-", 0x0A - 0B, "Reserved", "" },
	{ "VK_CLEAR", 0x0C, "Numpad 5", "" },
	{ "VK_RETURN", 0x0D, "Enter", "Numpad Enter" },
	//{ "-", 0x0E - 0F, "Undefined", "" },
	{ "VK_SHIFT", 0x10, "Shift", "" },
	{ "VK_CONTROL", 0x11, "Ctrl", "" },
	{ "VK_MENU", 0x12, "Alt", "" },
	{ "VK_PAUSE", 0x13, "Pause", "" },
	{ "VK_CAPITAL", 0x14, "Caps Lock", "" },
	{ "VK_KANA", 0x15, "IME Kana mode", "" },
	{ "VK_HANGUEL", 0x15, "IME Hanguel mode", "" },
	{ "VK_HANGUL", 0x15, "IME Hangul mode", "" },
	//{ "-", 0x16, "Undefined", "" },
	{ "VK_JUNJA", 0x17, "IME Junja mode", "" },
	{ "VK_FINAL", 0x18, "IME final mode", "" },
	{ "VK_HANJA", 0x19, "IME Hanja mode", "" },
	{ "VK_KANJI", 0x19, "IME Kanji mode", "" },
	//{ "-", 0x1A, "Undefined", "" },
	{ "VK_ESCAPE", 0x1B, "Esc", "" },
	{ "VK_CONVERT", 0x1C, "IME convert", "" },
	{ "VK_NONCONVERT", 0x1D, "IME nonconvert", "" },
	{ "VK_ACCEPT", 0x1E, "IME accept", "" },
	{ "VK_MODECHANGE", 0x1F, "IME mode change request", "" },
	{ "VK_SPACE", 0x20, "Spacebar", "" },
	{ "VK_PRIOR", 0x21, "Numpad 9", "Page Up" },
	{ "VK_NEXT", 0x22, "Numpad 3", "Page Down" },
	{ "VK_END", 0x23, "Numpad 1", "End" },
	{ "VK_HOME", 0x24, "Numpad 7", "Home" },
	{ "VK_LEFT", 0x25, "Numpad 4", "Left Arrow" },
	{ "VK_UP", 0x26, "Numpad 8", "Up Arrow" },
	{ "VK_RIGHT", 0x27, "Numpad 6", "Right Arrow" },
	{ "VK_DOWN", 0x28, "Numpad 2", "Down Arrow" },
	{ "VK_SELECT", 0x29, "Select", "" },
	{ "VK_PRINT", 0x2A, "Print", "" },
	{ "VK_EXECUTE", 0x2B, "Execute", "" },
	{ "VK_SNAPSHOT", 0x2C, "Print Screen", "" },
	{ "VK_INSERT", 0x2D, "Numpad 0", "Insert" },
	{ "VK_DELETE", 0x2E, "Numpad .", "Delete" },
	{ "VK_HELP", 0x2F, "Help", "" },
	{ "0", 0x30, "0", "" },
	{ "1", 0x31, "1", "" },
	{ "2", 0x32, "2", "" },
	{ "3", 0x33, "3", "" },
	{ "4", 0x34, "4", "" },
	{ "5", 0x35, "5", "" },
	{ "6", 0x36, "6", "" },
	{ "7", 0x37, "7", "" },
	{ "8", 0x38, "8", "" },
	{ "9", 0x39, "9", "" },
	//{ "undefined", 0x3A - 40, "undefined", "" },
	{ "A", 0x41, "A", "" },
	{ "B", 0x42, "B", "" },
	{ "C", 0x43, "C", "" },
	{ "D", 0x44, "D", "" },
	{ "E", 0x45, "E", "" },
	{ "F", 0x46, "F", "" },
	{ "G", 0x47, "G", "" },
	{ "H", 0x48, "H", "" },
	{ "I", 0x49, "I", "" },
	{ "J", 0x4A, "J", "" },
	{ "K", 0x4B, "K", "" },
	{ "L", 0x4C, "L", "" },
	{ "M", 0x4D, "M", "" },
	{ "N", 0x4E, "N", "" },
	{ "O", 0x4F, "O", "" },
	{ "P", 0x50, "P", "" },
	{ "Q", 0x51, "Q", "" },
	{ "R", 0x52, "R", "" },
	{ "S", 0x53, "S", "" },
	{ "T", 0x54, "T", "" },
	{ "U", 0x55, "U", "" },
	{ "V", 0x56, "V", "" },
	{ "W", 0x57, "W", "" },
	{ "X", 0x58, "X", "" },
	{ "Y", 0x59, "Y", "" },
	{ "Z", 0x5A, "Z", "" },
	{ "VK_LWIN", 0x5B, "Left Windows", "" },
	{ "VK_RWIN", 0x5C, "Right Windows", "" },
	{ "VK_APPS", 0x5D, "Applications Key", "" },
	//{ "-", 0x5E, "Reserved", "" },
	{ "VK_SLEEP", 0x5F, "Computer Sleep", "" },
	{ "VK_NUMPAD0", 0x60, "Keypad 0", "" },
	{ "VK_NUMPAD1", 0x61, "Keypad 1", "" },
	{ "VK_NUMPAD2", 0x62, "Keypad 2", "" },
	{ "VK_NUMPAD3", 0x63, "Keypad 3", "" },
	{ "VK_NUMPAD4", 0x64, "Keypad 4", "" },
	{ "VK_NUMPAD5", 0x65, "Keypad 5", "" },
	{ "VK_NUMPAD6", 0x66, "Keypad 6", "" },
	{ "VK_NUMPAD7", 0x67, "Keypad 7", "" },
	{ "VK_NUMPAD8", 0x68, "Keypad 8", "" },
	{ "VK_NUMPAD9", 0x69, "Keypad 9", "" },
	{ "VK_MULTIPLY", 0x6A, "Numpad *", "" },
	{ "VK_ADD", 0x6B, "Numpad +", "" },
	{ "VK_SEPARATOR", 0x6C, "Separator", "" },
	{ "VK_SUBTRACT", 0x6D, "Numpad -", "" },
	{ "VK_DECIMAL", 0x6E, "Decimal", "" },
	{ "VK_DIVIDE", 0x6F, "Numpad /", "" },
	{ "VK_F1", 0x70, "F1", "" },
	{ "VK_F2", 0x71, "F2", "" },
	{ "VK_F3", 0x72, "F3", "" },
	{ "VK_F4", 0x73, "F4", "" },
	{ "VK_F5", 0x74, "F5", "" },
	{ "VK_F6", 0x75, "F6", "" },
	{ "VK_F7", 0x76, "F7", "" },
	{ "VK_F8", 0x77, "F8", "" },
	{ "VK_F9", 0x78, "F9", "" },
	{ "VK_F10", 0x79, "F10", "" },
	{ "VK_F11", 0x7A, "F11", "" },
	{ "VK_F12", 0x7B, "F12", "" },
	{ "VK_F13", 0x7C, "F13", "" },
	{ "VK_F14", 0x7D, "F14", "" },
	{ "VK_F15", 0x7E, "F15", "" },
	{ "VK_F16", 0x7F, "F16", "" },
	{ "VK_F17", 0x80, "F17", "" },
	{ "VK_F18", 0x81, "F18", "" },
	{ "VK_F19", 0x82, "F19", "" },
	{ "VK_F20", 0x83, "F20", "" },
	{ "VK_F21", 0x84, "F21", "" },
	{ "VK_F22", 0x85, "F22", "" },
	{ "VK_F23", 0x86, "F23", "" },
	{ "VK_F24", 0x87, "F24", "" },
	//{ "-", 0x88 - 8F, "Unassigned", "" },
	{ "VK_NUMLOCK", 0x90, "Pause", "Num Lock" },
	{ "VK_SCROLL", 0x91, "Scroll Lock", "" },
	//{"-", 0x92-96,"OEM specific"},
	//{ "-", 0x97 - 9F, "Unassigned", "" },
	{ "VK_LSHIFT", 0xA0, "Left Shift", "" },
	{ "VK_RSHIFT", 0xA1, "Right Shift", "" },
	{ "VK_LCONTROL", 0xA2, "Left Control", "" },
	{ "VK_RCONTROL", 0xA3, "Right Control", "" },
	{ "VK_LMENU", 0xA4, "Left Menu", "" },
	{ "VK_RMENU", 0xA5, "Right Menu", "" },
	{ "VK_BROWSER_BACK", 0xA6, "Browser Back", "" },
	{ "VK_BROWSER_FORWARD", 0xA7, "Browser Forward", "" },
	{ "VK_BROWSER_REFRESH", 0xA8, "Browser Refresh", "" },
	{ "VK_BROWSER_STOP", 0xA9, "Browser Stop", "" },
	{ "VK_BROWSER_SEARCH", 0xAA, "Browser Search", "" },
	{ "VK_BROWSER_FAVORITES", 0xAB, "Browser Favorites", "" },
	{ "VK_BROWSER_HOME", 0xAC, "Browser Start and Home", "" },
	{ "VK_VOLUME_MUTE", 0xAD, "Volume Mute", "" },
	{ "VK_VOLUME_DOWN", 0xAE, "Volume Down", "" },
	{ "VK_VOLUME_UP", 0xAF, "Volume Up", "" },
	{ "VK_MEDIA_NEXT_TRACK", 0xB0, "Next Track", "" },
	{ "VK_MEDIA_PREV_TRACK", 0xB1, "Previous Track", "" },
	{ "VK_MEDIA_STOP", 0xB2, "Stop Media", "" },
	{ "VK_MEDIA_PLAY_PAUSE", 0xB3, "Play/Pause Media", "" },
	{ "VK_LAUNCH_MAIL", 0xB4, "Start Mail", "" },
	{ "VK_LAUNCH_MEDIA_SELECT", 0xB5, "Select Media", "" },
	{ "VK_LAUNCH_APP1", 0xB6, "Start Application 1", "" },
	{ "VK_LAUNCH_APP2", 0xB7, "Start Application 2", "" },
	//{ "-", 0xB8 - B9, "Reserved", "" },
	{ "VK_OEM_1", 0xBA, ";", "" },
	{ "VK_OEM_PLUS", 0xBB, "=", "" },
	{ "VK_OEM_COMMA", 0xBC, ",", "" },
	{ "VK_OEM_MINUS", 0xBD, "-", "" },
	{ "VK_OEM_PERIOD", 0xBE, ".", "" },
	{ "VK_OEM_2", 0xBF, "/", "Numpad /" },
	{ "VK_OEM_3", 0xC0, "`", "" },
	//{ "-", 0xC1 - D7, "Reserved", "" },
	//{ "-", 0xD8 - DA, "Unassigned", "" },
	{ "VK_OEM_4", 0xDB, "[", "" },
	{ "VK_OEM_5", 0xDC, "\\", "" },
	{ "VK_OEM_6", 0xDD, "]", "" },
	{ "VK_OEM_7", 0xDE, "'", "" },
	{ "VK_OEM_8", 0xDF, "Used for miscellaneous characters; it can vary by keyboard.", "" },
	//{ "-", 0xE0, "Reserved", "" },
	//{ "-", 0xE1, "OEM specific", "" },
	{ "VK_OEM_102", 0xE2, "Pipe", "" },
	//{ "-", 0xE3 - E4, "OEM specific", "" },
	{ "VK_PROCESSKEY", 0xE5, "IME PROCESS", "" },
	//{ "-", 0xE6, "OEM specific", "" },
	{ "VK_PACKET", 0xE7, "Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP", "" },
	//{ "-", 0xE8, "Unassigned", "" },
	//  {"-",0xE6,"OEM specific"},
	{ "VK_PACKET", 0xE7, "Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP", "" },
	//  {"-",0xE8,"Unassigned"},
	//{ "-", 0xE9 - F5, "OEM specific", "" },
	{ "VK_ATTN", 0xF6, "Attn", "" },
	{ "VK_CRSEL", 0xF7, "CrSel", "" },
	{ "VK_EXSEL", 0xF8, "ExSel", "" },
	{ "VK_EREOF", 0xF9, "Erase EOF", "Menu" },
	{ "VK_PLAY", 0xFA, "Play", "" },
	{ "VK_ZOOM", 0xFB, "Zoom", "" },
	{ "VK_NONAME", 0xFC, "Reserved", "" },
	{ "VK_PA1", 0xFD, "PA1", "" },
	{ "VK_OEM_CLEAR", 0xFE, "Clear", "" }
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
			_keyDefinitions.push_back({ "", (uint32_t)(0xFFFF + i * 0x100 + j + 1), "Pad" + std::to_string(i + 1) + " " + buttonNames[j] });
		}
	}

	//Init DirectInput buttons
	vector<string> diButtonNames = { "Y+", "Y-", "X-", "X+", "Y2+", "Y2-", "X2-", "X2+", "Z+", "Z-", "Z2+", "Z2-", "DPad Up", "DPad Down", "DPad Right", "DPad Left" };
	for(int i = 0; i < 16; i++) {
		for(int j = 0; j < (int)diButtonNames.size(); j++) {
			_keyDefinitions.push_back({ "", (uint32_t)(0x11000 + i * 0x100 + j), "Joy" + std::to_string(i + 1) + " " + diButtonNames[j] });
		}

		for(int j = 0; j < 128; j++) {
			_keyDefinitions.push_back({ "", (uint32_t)(0x11000 + i * 0x100 + j + 0x10), "Joy" + std::to_string(i + 1) + " But" + std::to_string(j+1)});
		}
	}

	for(KeyDefinition &keyDef : _keyDefinitions) {
		_keyNames[keyDef.keyCode] = keyDef.description;
		_keyExtendedNames[keyDef.keyCode] = keyDef.extDescription.empty() ? "Ext " + keyDef.description : keyDef.extDescription;
		
		uint32_t keyCode = keyDef.keyCode <= 0xFFFF ? MapVirtualKeyEx(keyDef.keyCode & 0xFF, MAPVK_VK_TO_VSC, nullptr) : keyDef.keyCode;
		_keyCodes[keyDef.description] = keyCode;
		if(!keyDef.extDescription.empty()) {
			_keyCodes[keyDef.extDescription] = 0x100 | keyCode;
		}
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
		for(int j = 0; j < 0x29; j++) {
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

string WindowsKeyManager::GetKeyName(uint32_t scanCode)
{
	uint32_t keyCode = scanCode <= 0xFFFF ? MapVirtualKeyEx(scanCode & 0xFF, MAPVK_VSC_TO_VK, nullptr) : scanCode;
	bool extendedKey = (scanCode <= 0xFFFF && scanCode & 0x100);
	auto keyDef = (extendedKey ? _keyExtendedNames : _keyNames).find(keyCode);
	if(keyDef != (extendedKey ? _keyExtendedNames : _keyNames).end()) {
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
	if(scanCode > 0x1FF) {
		_mouseState[scanCode & 0x03] = state;
	} else {
		uint32_t keyCode = MapVirtualKeyEx(scanCode & 0xFF, MAPVK_VSC_TO_VK, nullptr);
		if(keyCode >= 0x10 && keyCode <= 0x12) {
			//Ignore "ext" flag for alt, ctrl & shift
			scanCode = MapVirtualKeyEx(keyCode, MAPVK_VK_TO_VSC, nullptr);
		}
		_keyState[scanCode & 0x1FF] = state;
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