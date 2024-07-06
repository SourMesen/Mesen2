#pragma once
#include "pch.h"
#include "Shared/Interfaces/INotificationListener.h"
#include "Shared/SettingTypes.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/Timer.h"

class Emulator;

class ShortcutKeyHandler final : public INotificationListener, public std::enable_shared_from_this<ShortcutKeyHandler>
{
private:
	Emulator* _emu = nullptr;

	thread _thread;
	atomic<bool> _stopThread;
	SimpleLock _lock;
	
	int _keySetIndex = 0;
	vector<uint16_t> _pressedKeys;
	vector<uint16_t> _lastPressedKeys;
	bool _isKeyUp = false;
	bool _isKeyboardConnected = false;
	bool _isPaused = false;

	Timer _runSingleFrameRepeatTimer;
	atomic<bool> _repeatStarted;
	atomic<bool> _needRepeat;

	unordered_set<uint32_t> _keysDown[2];
	unordered_set<uint32_t> _prevKeysDown[2];
	
	void CheckMappedKeys();
	
	bool IsKeyPressed(EmulatorShortcut key);
	bool IsKeyPressed(KeyCombination comb, bool blockKeyboardKeys);
	bool IsKeyPressed(uint16_t keyCode, bool mergeCtrlAltShift, bool blockKeyboardKeys);

	bool DetectKeyPress(EmulatorShortcut key);
	bool DetectKeyRelease(EmulatorShortcut key);

	void ProcessRunSingleFrame();

	void ProcessShortcutPressed(EmulatorShortcut shortcut, uint32_t shortcutParam);
	void ProcessShortcutReleased(EmulatorShortcut shortcut, uint32_t shortcutParam);

public:
	ShortcutKeyHandler(Emulator* emu);
	virtual ~ShortcutKeyHandler();

	bool IsShortcutAllowed(EmulatorShortcut shortcut, uint32_t shortcutParam);
	void ProcessKeys();

	void ProcessNotification(ConsoleNotificationType type, void* parameter) override;
};
