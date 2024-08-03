#pragma once
#include "pch.h"

enum class MouseButton
{
	LeftButton = 0,
	RightButton = 1,
	MiddleButton = 2,
	Button4 = 3,
	Button5 = 4
};

struct MousePosition
{
	int16_t X;
	int16_t Y;
	double RelativeX;
	double RelativeY;
};

struct MouseMovement
{
	int16_t dx;
	int16_t dy;
};

class IKeyManager
{
public:
	static constexpr int BaseMouseButtonIndex = 0x200;
	static constexpr int BaseGamepadIndex = 0x1000;

	virtual ~IKeyManager() {}

	virtual void RefreshState() = 0;
	virtual void UpdateDevices() = 0;
	virtual bool IsMouseButtonPressed(MouseButton button) = 0;
	virtual bool IsKeyPressed(uint16_t keyCode) = 0;
	virtual optional<int16_t> GetAxisPosition(uint16_t keyCode) { return std::nullopt; }
	virtual vector<uint16_t> GetPressedKeys() = 0;
	virtual string GetKeyName(uint16_t keyCode) = 0;
	virtual uint16_t GetKeyCode(string keyName) = 0;

	virtual bool SetKeyState(uint16_t scanCode, bool state) = 0;
	virtual void ResetKeyState() = 0;
	virtual void SetDisabled(bool disabled) = 0;

	virtual void SetForceFeedback(uint16_t magnitude) {}
};