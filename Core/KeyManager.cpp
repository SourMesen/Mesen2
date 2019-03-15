#include "stdafx.h"
#include "KeyManager.h"
#include "IKeyManager.h"
#include "EmuSettings.h"
#include "Console.h"
#include "VideoDecoder.h"

IKeyManager* KeyManager::_keyManager = nullptr;
MousePosition KeyManager::_mousePosition = { 0, 0 };
atomic<int16_t> KeyManager::_xMouseMovement;
atomic<int16_t> KeyManager::_yMouseMovement;

void KeyManager::RegisterKeyManager(IKeyManager* keyManager)
{
	_xMouseMovement = 0;
	_yMouseMovement = 0;
	_keyManager = keyManager;
}

void KeyManager::RefreshKeyState()
{
	if(_keyManager != nullptr) {
		return _keyManager->RefreshState();
	}
}

bool KeyManager::IsKeyPressed(uint32_t keyCode)
{
	if(_keyManager != nullptr) {
		return _keyManager->IsKeyPressed(keyCode);
	}
	return false;
}

bool KeyManager::IsMouseButtonPressed(MouseButton button)
{
	if(_keyManager != nullptr) {
		//TODO return _settings->InputEnabled() && 
		return _keyManager->IsMouseButtonPressed(button);
	}
	return false;
}

vector<uint32_t> KeyManager::GetPressedKeys()
{
	if(_keyManager != nullptr) {
		return _keyManager->GetPressedKeys();
	}
	return vector<uint32_t>();
}

string KeyManager::GetKeyName(uint32_t keyCode)
{
	if(_keyManager != nullptr) {
		return _keyManager->GetKeyName(keyCode);
	}
	return "";
}

uint32_t KeyManager::GetKeyCode(string keyName)
{
	if(_keyManager != nullptr) {
		return _keyManager->GetKeyCode(keyName);
	}
	return 0;
}

void KeyManager::UpdateDevices()
{
	if(_keyManager != nullptr) {
		_keyManager->UpdateDevices();
	}
}

void KeyManager::SetMouseMovement(int16_t x, int16_t y)
{
	_xMouseMovement += x;
	_yMouseMovement += y;
}

MouseMovement KeyManager::GetMouseMovement(double videoScale, double mouseSensitivity)
{
	double factor = videoScale / mouseSensitivity;
	MouseMovement mov = {};
	mov.dx = (int16_t)(_xMouseMovement / factor);
	mov.dy = (int16_t)(_yMouseMovement / factor);
	_xMouseMovement -= (int16_t)(mov.dx * factor);
	_yMouseMovement -= (int16_t)(mov.dy * factor);

	return mov;
}

void KeyManager::SetMousePosition(shared_ptr<Console> console, double x, double y)
{
	if(x < 0 || y < 0) {
		_mousePosition.X = -1;
		_mousePosition.Y = -1;
	} else {
		OverscanDimensions overscan = console->GetSettings()->GetOverscan();
		ScreenSize size = console->GetVideoDecoder()->GetScreenSize(false);
		_mousePosition.X = (int32_t)(x*size.Width + overscan.Left);
		_mousePosition.Y = (int32_t)(y*size.Height + overscan.Top);
	}
}

MousePosition KeyManager::GetMousePosition()
{
	return _mousePosition;
}