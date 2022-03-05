#pragma once
#include "stdafx.h"
#include "SettingTypes.h"
#include "ControlDeviceState.h"

class Emulator;
class DebugHud;

class InputHud
{
private:
	Emulator* _emu;
	DebugHud* _hud;

	int _xOffset = 0;
	int _yOffset = 0;
	int _outlineWidth = 0;
	int _outlineHeight = 0;

	void DrawController(ControllerData& data);

public:
	InputHud(Emulator *emu, DebugHud* hud);

	void DrawOutline(int width, int height);
	void DrawButton(int x, int y, int width, int height, bool pressed);
	void DrawNumber(int number, int x, int y);
	void EndDrawController();

	void DrawControllers(FrameInfo size, vector<ControllerData> controllerData);
};