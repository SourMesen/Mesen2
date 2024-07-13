#include "Common.h"
#include "XInputManager.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/EmuSettings.h"

XInputManager::XInputManager(Emulator* emu)
{
	_emu = emu;
	for(int i = 0; i < XUSER_MAX_COUNT; i++) {
		_gamePadConnected[i] = true;
	}
}

void XInputManager::RefreshState()
{
	XINPUT_STATE state;
	for(DWORD i = 0; i < XUSER_MAX_COUNT; i++) {
		if(_gamePadConnected[i]) {
			if(XInputGetState(i, &state) != ERROR_SUCCESS) {
				//XInputGetState is incredibly slow when no controller is plugged in
				ZeroMemory(&_gamePadStates[i], sizeof(XINPUT_STATE));
				_gamePadConnected[i] = false;
			} else {
				_gamePadStates[i] = state;
			}
		}
	}
}

bool XInputManager::NeedToUpdate()
{
	for(int i = 0; i < XUSER_MAX_COUNT; i++) {
		if(!_gamePadConnected[i]) {
			XINPUT_STATE state;
			if(XInputGetState(i, &state) == ERROR_SUCCESS) {
				return true;
			}
		}
	}
	return false;
}

void XInputManager::UpdateDeviceList()
{
	//Periodically detect if a controller has been plugged in to allow controllers to be plugged in after the emu is started
	for(int i = 0; i < XUSER_MAX_COUNT; i++) {
		_gamePadConnected[i] = true;
	}
}

bool XInputManager::IsPressed(uint8_t gamepadPort, uint8_t button)
{
	if(_gamePadConnected[gamepadPort]) {
		XINPUT_GAMEPAD &gamepad = _gamePadStates[gamepadPort].Gamepad;
		bool pressed = false;
		if(button <= 16) {
			WORD xinputButton = 1 << (button - 1);
			pressed = (_gamePadStates[gamepadPort].Gamepad.wButtons & xinputButton) != 0;
		} else {
			double ratio = _emu->GetSettings()->GetControllerDeadzoneRatio() * 2;

			switch(button) {
				case 17: pressed = gamepad.bLeftTrigger > (XINPUT_GAMEPAD_TRIGGER_THRESHOLD * ratio); break;
				case 18: pressed = gamepad.bRightTrigger > (XINPUT_GAMEPAD_TRIGGER_THRESHOLD * ratio); break;
				case 19: pressed = gamepad.sThumbRY > (XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE * ratio); break;
				case 20: pressed = gamepad.sThumbRY < -(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE * ratio); break;
				case 21: pressed = gamepad.sThumbRX < -(XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE * ratio); break;
				case 22: pressed = gamepad.sThumbRX > (XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE * ratio); break;
				case 23: pressed = gamepad.sThumbLY > (XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE * ratio); break;
				case 24: pressed = gamepad.sThumbLY < -(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE * ratio); break;
				case 25: pressed = gamepad.sThumbLX < -(XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE * ratio); break;
				case 26: pressed = gamepad.sThumbLX > (XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE * ratio); break;
			}
		}

		_enableForceFeedback[gamepadPort] |= pressed;

		return pressed;
	}
	return false;
}

optional<int16_t> XInputManager::GetAxisPosition(uint8_t port, int axis)
{
	if(_gamePadConnected[port]) {
		XINPUT_GAMEPAD& gamepad = _gamePadStates[port].Gamepad;
		switch(axis - 27) {
			case 0: return gamepad.sThumbLY;
			case 1: return gamepad.sThumbLX;
			case 2: return gamepad.sThumbRY;
			case 3: return gamepad.sThumbRX;
		}
	}

	return std::nullopt;
}

void XInputManager::SetForceFeedback(uint16_t magnitude)
{
	XINPUT_VIBRATION settings = {};
	settings.wLeftMotorSpeed = magnitude;
	settings.wRightMotorSpeed = magnitude;

	for(int i = 0; i < XUSER_MAX_COUNT; i++) {
		if(_enableForceFeedback[i]) {
			XInputSetState(i, &settings);
		}
	}
}
