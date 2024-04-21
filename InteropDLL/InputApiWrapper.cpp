#include "Common.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/BaseControlManager.h"
#include "Core/Shared/KeyManager.h"
#include "Core/Shared/ShortcutKeyHandler.h"
#include "Utilities/StringUtilities.h"
#include "Core/Shared/Interfaces/IMouseManager.h"

extern unique_ptr<IKeyManager> _keyManager;
extern unique_ptr<IMouseManager> _mouseManager;
extern unique_ptr<Emulator> _emu;

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

	DllExport void __stdcall GetPressedKeys(uint16_t* keyBuffer)
	{
		vector<uint16_t> pressedKeys = KeyManager::GetPressedKeys();
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

	DllExport void __stdcall SetKeyState(uint16_t scanCode, bool state)
	{
		if(_keyManager) {
			if(_keyManager->SetKeyState(scanCode, state)) {
				_emu->GetShortcutKeyHandler()->ProcessKeys();
			}
		}
	}
	
	DllExport void __stdcall ResetKeyState()
	{
		if(_keyManager) {
			_keyManager->ResetKeyState();
		}
	}

	DllExport void __stdcall GetKeyName(uint16_t keyCode, char* outKeyName, uint32_t maxLength)
	{
		StringUtilities::CopyToBuffer(KeyManager::GetKeyName(keyCode), outKeyName, maxLength);
	}

	DllExport uint16_t __stdcall GetKeyCode(char* keyName)
	{
		if(keyName) {
			return KeyManager::GetKeyCode(keyName);
		} else {
			return 0;
		}
	}

	DllExport bool __stdcall HasControlDevice(ControllerType type)
	{
		return _emu->HasControlDevice(type);
	}

	DllExport void __stdcall ResetLagCounter()
	{
		_emu->ResetLagCounter();
	}

	DllExport SystemMouseState __stdcall GetSystemMouseState(void* rendererHandle)
	{
		if(_mouseManager) {
			return _mouseManager->GetSystemMouseState(rendererHandle);
		}
		SystemMouseState state = {};
		return state;
	}

	DllExport bool __stdcall CaptureMouse(int32_t x, int32_t y, int32_t width, int32_t height, void* rendererHandle)
	{
		if(_mouseManager) {
			return _mouseManager->CaptureMouse(x, y, width, height, rendererHandle);
		}
		return false;
	}

	DllExport void __stdcall ReleaseMouse()
	{
		if(_mouseManager) {
			_mouseManager->ReleaseMouse();
		}
	}

	DllExport void __stdcall SetSystemMousePosition(int32_t x, int32_t y)
	{
		if(_mouseManager) {
			_mouseManager->SetSystemMousePosition(x, y);
		}
	}

	DllExport void __stdcall SetCursorImage(CursorImage image)
	{
		if(_mouseManager) {
			_mouseManager->SetCursorImage(image);
		}
	}

	DllExport double __stdcall GetPixelScale()
	{
		if(_mouseManager) {
			return _mouseManager->GetPixelScale();
		}
		return 1.0;
	}
}
