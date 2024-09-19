#pragma once
#include "pch.h"
#include "Shared/SettingTypes.h"
#include "Utilities/SimpleLock.h"
#include "Utilities/ISerializable.h"
#include <random>

class Emulator;

class EmuSettings final : public ISerializable
{
private:
	Emulator* _emu;
	std::mt19937 _mt;

	VideoConfig _video;
	AudioConfig _audio;
	InputConfig _input;
	EmulationConfig _emulation;
	PreferencesConfig _preferences;
	AudioPlayerConfig _audioPlayer;
	DebugConfig _debug;

	GameConfig _game;

	SnesConfig _snes;
	GameboyConfig _gameboy;
	NesConfig _nes;
	PcEngineConfig _pce;
	SmsConfig _sms;
	CvConfig _cv;
	GbaConfig _gba;
	WsConfig _ws;

	atomic<uint32_t> _flags;
	atomic<uint64_t> _debuggerFlags;

	string _audioDevice;
	string _saveFolder;
	string _saveStateFolder;
	string _screenshotFolder;

	std::unordered_map<uint32_t, KeyCombination> _emulatorKeys[3];
	std::unordered_map<uint32_t, vector<KeyCombination>> _shortcutSupersets[3];

	SimpleLock _updateShortcutsLock;

	void ProcessString(string &str, const char** strPointer);

	void ClearShortcutKeys();
	void SetShortcutKey(EmulatorShortcut shortcut, KeyCombination keyCombination, int keySetIndex);

public:
	EmuSettings(Emulator* emu);
	
	void CopySettings(EmuSettings& src);

	void Serialize(Serializer& s) override;

	uint32_t GetVersion();
	string GetVersionString();

	void SetVideoConfig(VideoConfig& config);
	VideoConfig& GetVideoConfig();

	void SetAudioConfig(AudioConfig& config);
	AudioConfig& GetAudioConfig();

	void SetInputConfig(InputConfig& config);
	InputConfig& GetInputConfig();

	void SetEmulationConfig(EmulationConfig& config);
	EmulationConfig& GetEmulationConfig();

	void SetSnesConfig(SnesConfig& config);
	SnesConfig& GetSnesConfig();

	void SetNesConfig(NesConfig& config);
	NesConfig& GetNesConfig();

	void SetGameboyConfig(GameboyConfig& config);
	GameboyConfig& GetGameboyConfig();

	void SetGbaConfig(GbaConfig& config);
	GbaConfig& GetGbaConfig();

	void SetPcEngineConfig(PcEngineConfig& config);
	PcEngineConfig& GetPcEngineConfig();

	void SetSmsConfig(SmsConfig& config);
	SmsConfig& GetSmsConfig();

	void SetCvConfig(CvConfig& config);
	CvConfig& GetCvConfig();

	void SetWsConfig(WsConfig& config);
	WsConfig& GetWsConfig();

	void SetGameConfig(GameConfig& config);
	GameConfig& GetGameConfig();

	void SetPreferences(PreferencesConfig& config);
	PreferencesConfig& GetPreferences();
	
	void SetAudioPlayerConfig(AudioPlayerConfig& config);
	AudioPlayerConfig& GetAudioPlayerConfig();

	void SetDebugConfig(DebugConfig& config);
	DebugConfig& GetDebugConfig();

	void SetShortcutKeys(vector<ShortcutKeyInfo> shortcuts);
	KeyCombination GetShortcutKey(EmulatorShortcut shortcut, int keySetIndex);
	vector<KeyCombination> GetShortcutSupersets(EmulatorShortcut shortcut, int keySetIndex);

	OverscanDimensions GetOverscan();
	uint32_t GetEmulationSpeed();
	double GetAspectRatio(ConsoleRegion region, FrameInfo baseFrameSize);

	void SetFlag(EmulationFlags flag);
	void SetFlagState(EmulationFlags flag, bool enabled);
	void ClearFlag(EmulationFlags flag);
	bool CheckFlag(EmulationFlags flag);

	void SetDebuggerFlag(DebuggerFlags flag, bool enabled);
	bool CheckDebuggerFlag(DebuggerFlags flags);

	bool HasRandomPowerOnState(ConsoleType consoleType);
	int GetRandomValue(int maxValue);
	bool GetRandomBool();
	void InitializeRam(RamState state, void* data, uint32_t length);

	bool IsInputEnabled();
	double GetControllerDeadzoneRatio();

	template<typename T>
	bool IsEqual(T& prevCfg, T& newCfg)
	{
		if(memcmp(&prevCfg, &newCfg, sizeof(T)) == 0) {
			return true;
		}
		memcpy(&prevCfg, &newCfg, sizeof(T));
		return false;
	}
};