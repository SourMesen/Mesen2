#pragma once
#include "stdafx.h"
#include "SettingTypes.h"
#include "ControlDeviceState.h"

class Console;

class InputHud
{
private:
	Console* _console;

	void DrawController(int port, ControlDeviceState state, int x, int y, int frameNumber);

public:
	InputHud(Console *console);

	void DrawControllers(OverscanDimensions overscan, int frameNumber);
};