#pragma once
#include "pch.h"
#include "Shared/Interfaces/IKeyManager.h"
#include "Utilities/SimpleLock.h"

class Emulator;
class EmuSettings;

class KeyManager
{
private:
	static IKeyManager* _keyManager;
	static MousePosition _mousePosition;
	static double _xMouseMovement;
	static double _yMouseMovement;
	static EmuSettings* _settings;
	static SimpleLock _lock;

public:
	static void RegisterKeyManager(IKeyManager* keyManager);
	static void SetSettings(EmuSettings* settings);

	static void RefreshKeyState();
	static bool IsKeyPressed(uint16_t keyCode);
	static optional<int16_t> GetAxisPosition(uint16_t keyCode);
	static bool IsMouseButtonPressed(MouseButton button);
	static vector<uint16_t> GetPressedKeys();
	static string GetKeyName(uint16_t keyCode);
	static uint16_t GetKeyCode(string keyName);

	static void UpdateDevices();
	
	static void SetMouseMovement(int16_t x, int16_t y);
	static MouseMovement GetMouseMovement(Emulator* emu, uint32_t mouseSensitivity);
	
	static void SetMousePosition(Emulator* emu, double x, double y);
	static MousePosition GetMousePosition();

	static void SetForceFeedback(uint16_t magnitude);
};