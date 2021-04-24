#pragma once
#include "stdafx.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/Timer.h"
#include "SettingTypes.h"

class Emulator;

class ShortcutKeyHandler
{
private:
	shared_ptr<Emulator> _emu;

	thread _thread;
	atomic<bool> _stopThread;
	SimpleLock _lock;
	
	int _keySetIndex;
	vector<uint32_t> _pressedKeys;
	vector<uint32_t> _lastPressedKeys;
	bool _isKeyUp;

	shared_ptr<Timer> _runSingleFrameRepeatTimer;
	bool _repeatStarted;

	unordered_set<uint32_t> _keysDown[2];
	unordered_set<uint32_t> _prevKeysDown[2];
	
	void CheckMappedKeys();
	
	bool IsKeyPressed(EmulatorShortcut key);
	bool IsKeyPressed(KeyCombination comb);
	bool IsKeyPressed(uint32_t keyCode);

	bool DetectKeyPress(EmulatorShortcut key);
	bool DetectKeyRelease(EmulatorShortcut key);

	void ProcessRunSingleFrame();

public:
	ShortcutKeyHandler(shared_ptr<Emulator> emu);
	~ShortcutKeyHandler();

	void ProcessKeys();
};
