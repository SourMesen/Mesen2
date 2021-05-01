#pragma once
#include "stdafx.h"
#include "SettingTypes.h"
#include <random>

class Emulator;

class EmuSettings
{
private:
	Emulator* _emu;
	std::mt19937 _mt;

	VideoConfig _video;
	AudioConfig _audio;
	InputConfig _input;
	EmulationConfig _emulation;
	GameboyConfig _gameboy;
	PreferencesConfig _preferences;
	AudioPlayerConfig _audioPlayer;
	SnesConfig _snes;
	NesConfig _nes;

	atomic<uint32_t> _flags;
	atomic<uint32_t> _inputConfigVersion;

	atomic<uint32_t> _debuggerFlags;

	string _audioDevice;
	string _saveFolder;
	string _saveStateFolder;
	string _screenshotFolder;

	std::unordered_map<uint32_t, KeyCombination> _emulatorKeys[3];
	std::unordered_map<uint32_t, vector<KeyCombination>> _shortcutSupersets[3];

	void ProcessString(string &str, const char** strPointer);

	void ClearShortcutKeys();
	void SetShortcutKey(EmulatorShortcut shortcut, KeyCombination keyCombination, int keySetIndex);

public:
	EmuSettings(Emulator* emu);

	uint32_t GetVersion();
	string GetVersionString();

	void SetVideoConfig(VideoConfig config);
	VideoConfig GetVideoConfig();

	void SetAudioConfig(AudioConfig config);
	AudioConfig GetAudioConfig();

	void SetInputConfig(InputConfig config);
	InputConfig GetInputConfig();
	uint32_t GetInputConfigVersion();

	void SetEmulationConfig(EmulationConfig config);
	EmulationConfig GetEmulationConfig();

	void SetSnesConfig(SnesConfig config);
	SnesConfig& GetSnesConfig();

	void SetNesConfig(NesConfig config);
	NesConfig& GetNesConfig();

	void SetGameboyConfig(GameboyConfig config);
	GameboyConfig GetGameboyConfig();

	void SetPreferences(PreferencesConfig config);
	PreferencesConfig GetPreferences();
	
	void SetAudioPlayerConfig(AudioPlayerConfig config);
	AudioPlayerConfig GetAudioPlayerConfig();

	void SetShortcutKeys(vector<ShortcutKeyInfo> shortcuts);
	KeyCombination GetShortcutKey(EmulatorShortcut shortcut, int keySetIndex);
	vector<KeyCombination> GetShortcutSupersets(EmulatorShortcut shortcut, int keySetIndex);

	OverscanDimensions GetOverscan();
	uint32_t GetRewindBufferSize();
	uint32_t GetEmulationSpeed();
	double GetAspectRatio(ConsoleRegion region);

	void SetFlag(EmulationFlags flag);
	void SetFlagState(EmulationFlags flag, bool enabled);
	void ClearFlag(EmulationFlags flag);
	bool CheckFlag(EmulationFlags flag);

	void SetDebuggerFlag(DebuggerFlags flag, bool enabled);
	bool CheckDebuggerFlag(DebuggerFlags flags);
	
	int GetRandomValue(int maxValue);
	bool GetRandomBool();
	void InitializeRam(void* data, uint32_t length);

	bool IsInputEnabled();
	double GetControllerDeadzoneRatio();
};