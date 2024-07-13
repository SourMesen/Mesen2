#pragma once

#include "pch.h"
#include "Common.h"
#include <Xinput.h>

class Emulator;

class XInputManager
{
	private:
		Emulator* _emu = nullptr;
		XINPUT_STATE _gamePadStates[XUSER_MAX_COUNT] = {};
		uint8_t _gamePadConnected[XUSER_MAX_COUNT] = {};
		bool _enableForceFeedback[XUSER_MAX_COUNT] = {};

	public:
		XInputManager(Emulator* emu);

		bool NeedToUpdate();
		void UpdateDeviceList();
		void RefreshState();
		bool IsPressed(uint8_t gamepadPort, uint8_t button);
		optional<int16_t> GetAxisPosition(uint8_t gamepadPort, int axis);

		void SetForceFeedback(uint16_t magnitude);
};
