#pragma once
#include "pch.h"
#include "Shared/SettingTypes.h"
#include "Shared/ControlDeviceState.h"
#include "Shared/Interfaces/IKeyManager.h"

class Emulator;
class DebugHud;
class BaseControlManager;

class InputHud
{
private:
	Emulator* _emu = nullptr;
	DebugHud* _hud = nullptr;

	int _xOffset = 0;
	int _yOffset = 0;
	int _outlineWidth = 0;
	int _outlineHeight = 0;
	int _controllerIndex = 0;

	void DrawController(ControllerData& data, BaseControlManager* controlManager);

public:
	InputHud(Emulator *emu, DebugHud* hud);

	void DrawMousePosition(MousePosition pos);
	void DrawOutline(int width, int height);
	void DrawButton(int x, int y, int width, int height, bool pressed);
	void DrawNumber(int number, int x, int y);
	void EndDrawController();

	int GetControllerIndex() { return _controllerIndex; }

	void DrawControllers(FrameInfo size, vector<ControllerData> controllerData);
};