#pragma once

#include "Common.h"
#include "Core/Shared/Interfaces/IKeyManager.h"
#include "Utilities/Timer.h"
#include "Utilities/AutoResetEvent.h"
#include "XInputManager.h"
#include "DirectInputManager.h"
#include "Shared/KeyDefinitions.h"

class Emulator;

class WindowsKeyManager : public IKeyManager
{
private:
	static constexpr int BaseDirectInputIndex = 0x2000;

	HWND _hWnd;
	Emulator* _emu;
	
	vector<KeyDefinition> _keyDefinitions;

	bool _keyState[0x205];
	unique_ptr<DirectInputManager> _directInput;
	unique_ptr<XInputManager> _xInput;
	unordered_map<uint16_t, string> _keyNames;
	unordered_map<string, uint16_t> _keyCodes;

	AutoResetEvent _stopSignal;

	thread _updateDeviceThread;
	atomic<bool> _stopUpdateDeviceThread = false;
	bool _disableAllKeys = false;

	void StartUpdateDeviceThread();

public:
	WindowsKeyManager(Emulator* emu, HWND hWnd);
	~WindowsKeyManager();

	void RefreshState() override;
	bool IsKeyPressed(uint16_t key) override;
	optional<int16_t> GetAxisPosition(uint16_t key) override;
	bool IsMouseButtonPressed(MouseButton button) override;
	vector<uint16_t> GetPressedKeys() override;
	string GetKeyName(uint16_t key) override;
	uint16_t GetKeyCode(string keyName) override;

	bool SetKeyState(uint16_t scanCode, bool state) override;
	void ResetKeyState() override;
	void SetDisabled(bool disabled) override;

	void SetForceFeedback(uint16_t magnitude) override;

	void UpdateDevices() override;
};