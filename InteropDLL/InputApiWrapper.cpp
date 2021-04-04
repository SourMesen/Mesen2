#include "stdafx.h"
#include "../Core/Emulator.h"
#include "../Core/KeyManager.h"
#include "../Core/ShortcutKeyHandler.h"

extern unique_ptr<IKeyManager> _keyManager;
extern unique_ptr<ShortcutKeyHandler> _shortcutKeyHandler;
extern shared_ptr<Emulator> _emu;

static string _returnString;

extern "C" 
{
	DllExport void __stdcall SetMousePosition(double x, double y)
	{
		KeyManager::SetMousePosition(_emu.get(), x, y);
	}

	DllExport void __stdcall SetMouseMovement(int16_t x, int16_t y)
	{
		KeyManager::SetMouseMovement(x, y);
	}

	DllExport void __stdcall UpdateInputDevices()
	{ 
		if(_keyManager) {
			_keyManager->UpdateDevices(); 
		} 
	}

	DllExport void __stdcall GetPressedKeys(uint32_t *keyBuffer)
	{
		vector<uint32_t> pressedKeys = KeyManager::GetPressedKeys();
		for(size_t i = 0; i < pressedKeys.size() && i < 3; i++) {
			keyBuffer[i] = pressedKeys[i];
		}
	}

	DllExport void __stdcall DisableAllKeys(bool disabled)
	{
		if(_keyManager) {
			_keyManager->SetDisabled(disabled);
		}
	}

	DllExport void __stdcall SetKeyState(int32_t scanCode, bool state)
	{
		if(_keyManager) {
			_keyManager->SetKeyState(scanCode, state);
			_shortcutKeyHandler->ProcessKeys();
		}
	}
	
	DllExport void __stdcall ResetKeyState()
	{
		if(_keyManager) {
			_keyManager->ResetKeyState();
		}
	}

	DllExport const char* __stdcall GetKeyName(uint32_t keyCode)
	{
		_returnString = KeyManager::GetKeyName(keyCode);
		return _returnString.c_str();
	}

	DllExport uint32_t __stdcall GetKeyCode(char* keyName)
	{
		if(keyName) {
			return KeyManager::GetKeyCode(keyName);
		} else {
			return 0;
		}
	}
}