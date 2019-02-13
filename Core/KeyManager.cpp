#include "stdafx.h"
#include "KeyManager.h"
#include "IKeyManager.h"

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
	//TODO
	/*if(_keyManager != nullptr) {
		return _settings->InputEnabled() && _keyManager->IsKeyPressed(keyCode);
	}*/
	return false;
}

bool KeyManager::IsMouseButtonPressed(MouseButton button)
{
	//TODO
	/*if(_keyManager != nullptr) {
		return _settings->InputEnabled() && _keyManager->IsMouseButtonPressed(button);
	}*/
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

MouseMovement KeyManager::GetMouseMovement(double mouseSensitivity)
{
	//TODO
	//double factor = _settings->GetVideoScale() / mouseSensitivity;
	MouseMovement mov = {};
	/*mov.dx = (int16_t)(_xMouseMovement / factor);
	mov.dy = (int16_t)(_yMouseMovement / factor);
	_xMouseMovement -= (int16_t)(mov.dx * factor);
	_yMouseMovement -= (int16_t)(mov.dy * factor);*/

	return mov;
}

void KeyManager::SetMousePosition(double x, double y)
{
	//TODO
	/*if(x < 0 || y < 0) {
		_mousePosition.X = -1;
		_mousePosition.Y = -1;
	} else {
		OverscanDimensions overscan = _settings->GetOverscanDimensions();
		_mousePosition.X = (int32_t)(x * (PPU::ScreenWidth - overscan.Left - overscan.Right) + overscan.Left);
		_mousePosition.Y = (int32_t)(y * (PPU::ScreenHeight - overscan.Top - overscan.Bottom) + overscan.Top);
	}*/
}

MousePosition KeyManager::GetMousePosition()
{
	return _mousePosition;
}