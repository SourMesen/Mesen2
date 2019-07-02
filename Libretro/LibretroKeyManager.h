#pragma once
#include "libretro.h"
#include "../Core/IKeyManager.h"
#include "../Core/KeyManager.h"
#include "../Core/Console.h"

class LibretroKeyManager : public IKeyManager
{
private:
	shared_ptr<Console> _console;
	retro_input_state_t _getInputState = nullptr;
	retro_input_poll_t _pollInput = nullptr;
	bool _mouseButtons[3] = { false, false, false };
	bool _wasPushed[16] = { };

	bool ProcessAction(uint32_t button)
	{
		if(_getInputState(0, RETRO_DEVICE_JOYPAD, 0, button)) {
			if(!_wasPushed[button]) {
				//Newly pressed, process action
				_wasPushed[button] = true;
				return true;
			}
		} else {
			_wasPushed[button] = false;
		}
		return false;
	}

public:
	LibretroKeyManager(shared_ptr<Console> console)
	{
		_console = console;
		KeyManager::RegisterKeyManager(this);
	}

	~LibretroKeyManager()
	{
		KeyManager::RegisterKeyManager(nullptr);
	}

	void SetGetInputState(retro_input_state_t getInputState)
	{
		_getInputState = getInputState;
	}

	void SetPollInput(retro_input_poll_t pollInput)
	{
		_pollInput = pollInput;
	}

	// Inherited via IKeyManager
	virtual void RefreshState() override
	{
		if(_pollInput) {
			_pollInput();
		}

		if(_getInputState) {
			int32_t x = _getInputState(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X);
			int32_t y = _getInputState(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y);

			x += 0x8000;
			y += 0x8000;

			KeyManager::SetMousePosition(_console, (double)x / 0x10000, (double)y / 0x10000);

			int16_t dx = _getInputState(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_X);
			int16_t dy = _getInputState(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_Y);
			KeyManager::SetMouseMovement(dx, dy);
			
			_mouseButtons[(int)MouseButton::LeftButton] = _getInputState(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_LEFT) != 0;
			_mouseButtons[(int)MouseButton::RightButton] = _getInputState(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_RIGHT) != 0;
			_mouseButtons[(int)MouseButton::MiddleButton] = _getInputState(0, RETRO_DEVICE_MOUSE, 0, RETRO_DEVICE_ID_MOUSE_MIDDLE) != 0;
		}
	}

	virtual bool IsKeyPressed(uint32_t keyCode) override
	{
		if(keyCode > 0 && _getInputState) {
			return _getInputState(keyCode >> 8, RETRO_DEVICE_JOYPAD, 0, (keyCode - 1) & 0xFF) != 0;
		} else {
			return false;
		}
	}

	virtual void UpdateDevices() override
	{
	}

	virtual bool IsMouseButtonPressed(MouseButton button) override
	{
		return _mouseButtons[(int)button];
	}

	virtual vector<uint32_t> GetPressedKeys() override
	{
		return vector<uint32_t>();
	}
	
	virtual string GetKeyName(uint32_t keyCode) override
	{
		return string();
	}

	virtual uint32_t GetKeyCode(string keyName) override
	{
		return 0;
	}

	virtual void SetKeyState(uint16_t scanCode, bool state) override
	{
	}

	virtual void ResetKeyState() override
	{
	}

	virtual void SetDisabled(bool disabled) override
	{
	}
};