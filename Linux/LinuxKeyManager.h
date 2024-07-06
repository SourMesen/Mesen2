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

	void RefreshState();
	bool IsKeyPressed(uint16_t key);
	bool IsMouseButtonPressed(MouseButton button);
	std::vector<uint16_t> GetPressedKeys();
	string GetKeyName(uint16_t key);
	uint16_t GetKeyCode(string keyName);

	void UpdateDevices();
	bool SetKeyState(uint16_t scanCode, bool state);
	void ResetKeyState();

	void SetDisabled(bool disabled);
};
