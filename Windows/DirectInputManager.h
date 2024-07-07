#pragma once
#include "Common.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include "Utilities/SimpleLock.h"

class Emulator;

struct DirectInputData
{
	LPDIRECTINPUTDEVICE8 joystick;
	DIJOYSTATE2 state;
	DIJOYSTATE2 defaultState;
	bool stateValid;
	DIDEVICEINSTANCE instanceInfo;
};

class DirectInputManager
{
private:
	static HWND _hWnd;
	Emulator* _emu;
	bool _needToUpdate = false;
	bool _requestUpdate = false;
	static LPDIRECTINPUT8 _directInput;
	static vector<DirectInputData> _joysticks;
	static vector<DirectInputData> _joysticksToAdd;

	static std::vector<GUID> _processedGuids;
	static std::vector<GUID> _xinputDeviceGuids;

	void Initialize();
	void UpdateInputState(DirectInputData& joystick);
	static bool ProcessDevice(const DIDEVICEINSTANCE* pdidInstance);
	static bool IsXInputDevice(const GUID* pGuidProductFromDirectInput);
	static int __stdcall EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, void* pContext);
	static int __stdcall EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, void* pContext);

public:
	DirectInputManager(Emulator* emu, HWND window);
	~DirectInputManager();

	void RefreshState();
	void UpdateDeviceList();
	int GetJoystickCount();
	bool IsPressed(int port, int button);
	optional<int16_t> GetAxisPosition(int port, int axis);
};
