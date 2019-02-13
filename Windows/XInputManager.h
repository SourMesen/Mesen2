#pragma once

#include "stdafx.h"
#include <Xinput.h>

class Console;

class XInputManager
{
	private:
		shared_ptr<Console> _console;
		vector<shared_ptr<XINPUT_STATE>> _gamePadStates;
		vector<uint8_t> _gamePadConnected;

	public:
		XInputManager(shared_ptr<Console> console);

		bool NeedToUpdate();
		void UpdateDeviceList();
		void RefreshState();
		bool IsPressed(uint8_t gamepadPort, uint8_t button);
};
