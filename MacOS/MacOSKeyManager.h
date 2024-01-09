#pragma once
#include <unordered_map>
#include <vector>
#include <thread>
#include "Utilities/AutoResetEvent.h"
#include "Shared/Interfaces/IKeyManager.h"
#include "Shared/KeyDefinitions.h"

class MacOSKeyManager : public IKeyManager
{
private:
	static constexpr int BaseMouseButtonIndex = 0x200;
	static constexpr int BaseGamepadIndex = 0x1000;

	vector<KeyDefinition> _keyDefinitions;
	bool _keyState[0x205];
	std::unordered_map<uint16_t, string> _keyNames;
	std::unordered_map<string, uint16_t> _keyCodes;

	bool _disableAllKeys;

	void* _eventMonitor;
	bool _disableHandling;

	//Mapping of MacOS keycodes to Avalonia keycodes
	//TODO Only had keys available on laptops tested (no numpad-keys etc)
	uint16_t _keyCodeMap[128] = {
		 44,  62,  47,  49,  51,  50,  69,  67,  46,  65,
		154,  45,  60,  66,  48,  61,  68,  63,  35,  36,
		 37,  38,  40,  39, 141,  43,  41, 143,  42,  34,
		151,  58,  64, 149,  52,  59,   6,  55,  53, 152,
		 54, 140, 150, 142, 145,  57,  56, 144,   3,  18,
		146,   2,   4,  13,  71,  70, 116,   8, 120, 118,
		117, 121, 119, 156, 106,  88,   0,  84,   0,  85,
		  0, 114, 131, 130, 129,  89,   4,   0,  87, 107,
		108,   4,  74,  75,  76,  77,  78,  79,  80,  81,
		  0,  82,  83, 163, 162,  86,  94,  95,  96,  92,
		 97,  98,  10, 100,   9, 102, 105, 103,   0,  99,
		 29, 101,   0, 104,  31,  22,  19,  32,  93,  21,
		 91,  20,  90,  23,  25,  26,  24, 156
	};

	void HandleModifiers(uint32_t flags);

public:
	MacOSKeyManager();
	virtual ~MacOSKeyManager();

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

	void SetLocalHandlingDisabled(bool disabled);
};
