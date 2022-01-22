#pragma once

#include "stdafx.h"
#include <Xinput.h>

class Emulator;

class XInputManager
{
	private:
		Emulator* _emu;
		vector<unique_ptr<XINPUT_STATE>> _gamePadStates;
		vector<uint8_t> _gamePadConnected;

	public:
		XInputManager(Emulator* emu);

		bool NeedToUpdate();
		void UpdateDeviceList();
		void RefreshState();
		bool IsPressed(uint8_t gamepadPort, uint8_t button);
};
