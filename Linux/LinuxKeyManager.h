#pragma once
#include <unordered_map>
#include <vector>
#include <thread>
#include "Utilities/AutoResetEvent.h"
#include "Shared/Interfaces/IKeyManager.h"
#include "Shared/KeyDefinitions.h"

class LinuxGameController;
class Emulator;

class LinuxKeyManager : public IKeyManager
{
private:
	Emulator* _emu;
	std::vector<shared_ptr<LinuxGameController>> _controllers;

	vector<KeyDefinition> _keyDefinitions;
	bool _keyState[0x205];
	std::unordered_map<uint16_t, string> _keyNames;
	std::unordered_map<string, uint16_t> _keyCodes;

	std::thread _updateDeviceThread;
	atomic<bool> _stopUpdateDeviceThread; 
	AutoResetEvent _stopSignal;
	bool _disableAllKeys;

	void StartUpdateDeviceThread();
	void CheckForGamepads(bool logInformation);

public:
	LinuxKeyManager(Emulator* emu);
	virtual ~LinuxKeyManager();

	void RefreshState() override;
	bool IsKeyPressed(uint16_t key) override;
	optional<int16_t> GetAxisPosition(uint16_t key) override;
	bool IsMouseButtonPressed(MouseButton button) override;
	std::vector<uint16_t> GetPressedKeys() override;
	string GetKeyName(uint16_t key) override;
	uint16_t GetKeyCode(string keyName) override;

	void UpdateDevices() override;
	bool SetKeyState(uint16_t scanCode, bool state) override;
	void ResetKeyState() override;

	void SetDisabled(bool disabled) override;

	void SetForceFeedback(uint16_t magnitude) override;
};
