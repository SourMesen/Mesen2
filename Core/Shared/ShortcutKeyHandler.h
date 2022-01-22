#pragma once
#include "stdafx.h"
#include "Shared/Interfaces/INotificationListener.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/Timer.h"
#include "SettingTypes.h"

class Emulator;

class ShortcutKeyHandler final : public INotificationListener, public std::enable_shared_from_this<ShortcutKeyHandler>
{
private:
	Emulator* _emu;

	thread _thread;
	atomic<bool> _stopThread;
	SimpleLock _lock;
	
	int _keySetIndex;
	vector<uint32_t> _pressedKeys;
	vector<uint32_t> _lastPressedKeys;
	bool _isKeyUp;

	Timer _runSingleFrameRepeatTimer;
	atomic<bool> _repeatStarted;
	atomic<bool> _needRepeat;

	unordered_set<uint32_t> _keysDown[2];
	unordered_set<uint32_t> _prevKeysDown[2];
	
	void CheckMappedKeys();
	
	bool IsKeyPressed(EmulatorShortcut key);
	bool IsKeyPressed(KeyCombination comb);
	bool IsKeyPressed(uint32_t keyCode, bool mergeCtrlAltShift);

	bool DetectKeyPress(EmulatorShortcut key);
	bool DetectKeyRelease(EmulatorShortcut key);

	void ProcessRunSingleFrame();

	void ProcessShortcutPressed(EmulatorShortcut shortcut);
	void ProcessShortcutReleased(EmulatorShortcut shortcut);

public:
	ShortcutKeyHandler(Emulator* emu);
	virtual ~ShortcutKeyHandler();

	bool IsShortcutAllowed(EmulatorShortcut shortcut);
	void ProcessKeys();

	void ProcessNotification(ConsoleNotificationType type, void* parameter) override;
};
