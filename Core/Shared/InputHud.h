#pragma once
#include "stdafx.h"
#include "SettingTypes.h"
#include "ControlDeviceState.h"

class Emulator;

class InputHud
{
private:
	Emulator* _emu;

	void DrawController(int port, ControlDeviceState state, int x, int y, int frameNumber);

public:
	InputHud(Emulator *emu);

	void DrawControllers(OverscanDimensions overscan, int frameNumber);
};