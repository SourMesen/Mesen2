#include "stdafx.h"
#include <random>
#include "EmuSettings.h"
#include "KeyManager.h"
#include "MessageManager.h"
#include "Console.h"
#include "SpcDisUtils.h"
#include "NotificationManager.h"
#include "../Utilities/FolderUtilities.h"

EmuSettings::EmuSettings(Console* console)
{
	_console = console;
	_flags = 0;
	_debuggerFlags = 0;
	_inputConfigVersion = 0;

	std::random_device rd;
	_mt = std::mt19937(rd());
}

uint32_t EmuSettings::GetVersion()
{
	//Version 0.3.0
	uint16_t major = 0;
	uint8_t minor = 3;
	uint8_t revision = 0;
	return (major << 16) | (minor << 8) | revision;
}

string EmuSettings::GetVersionString()
{
	uint32_t version = GetVersion();
	return std::to_string(version >> 16) + "." + std::to_string((version >> 8) & 0xFF) + "." + std::to_string(version & 0xFF);
}

void EmuSettings::ProcessString(string & str, const char ** strPointer)
{
	//Make a copy of the string and keep it (the original pointer will not be valid after the call is over)
	if(*strPointer) {
		str = *strPointer;
	} else {
		str.clear();
	}
	*strPointer = str.c_str();
}

void EmuSettings::SetVideoConfig(VideoConfig config)
{
	_video = config;
}

VideoConfig EmuSettings::GetVideoConfig()
{
	return _video;
}

void EmuSettings::SetAudioConfig(AudioConfig config)
{
	ProcessString(_audioDevice, &config.AudioDevice);

	_audio = config;
}

AudioConfig EmuSettings::GetAudioConfig()
{
	return _audio;
}

void EmuSettings::SetInputConfig(InputConfig config)
{
	bool controllersChanged = false;
	for(int i = 0; i < 5; i++) {
		controllersChanged |= _input.Controllers[i].Type != config.Controllers[i].Type;
	}

	_input = config;
	_inputConfigVersion++;

	if(controllersChanged) {
		//Used by net play
		_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::ConfigChanged);
	}
}

InputConfig EmuSettings::GetInputConfig()
{
	return _input;
}

uint32_t EmuSettings::GetInputConfigVersion()
{
	return _inputConfigVersion;
}

void EmuSettings::SetEmulationConfig(EmulationConfig config)
{
	bool prevOverclockEnabled = _emulation.PpuExtraScanlinesAfterNmi > 0 || _emulation.PpuExtraScanlinesBeforeNmi > 0 || _emulation.GsuClockSpeed > 100;
	bool overclockEnabled = config.PpuExtraScanlinesAfterNmi > 0 || config.PpuExtraScanlinesBeforeNmi > 0 || config.GsuClockSpeed > 100;
	_emulation = config;

	if(prevOverclockEnabled != overclockEnabled) {
		if(overclockEnabled) {
			MessageManager::DisplayMessage("Overclock", "OverclockEnabled");
		} else {
			MessageManager::DisplayMessage("Overclock", "OverclockDisabled");
		}

		//Used by net play
		_console->GetNotificationManager()->SendNotification(ConsoleNotificationType::ConfigChanged);
	}
}

EmulationConfig EmuSettings::GetEmulationConfig()
{
	return _emulation;
}

void EmuSettings::SetPreferences(PreferencesConfig config)
{
	ProcessString(_saveFolder, &config.SaveFolderOverride);
	ProcessString(_saveStateFolder, &config.SaveStateFolderOverride);
	ProcessString(_screenshotFolder, &config.ScreenshotFolderOverride);

	MessageManager::SetOsdState(!config.DisableOsd);

	FolderUtilities::SetFolderOverrides(
		_saveFolder,
		_saveStateFolder,
		_screenshotFolder,
		""
	);

	_preferences = config;
}

PreferencesConfig EmuSettings::GetPreferences()
{
	return _preferences;
}

void EmuSettings::ClearShortcutKeys()
{
	_emulatorKeys[0].clear();
	_emulatorKeys[1].clear();
	_emulatorKeys[2].clear();
	_shortcutSupersets[0].clear();
	_shortcutSupersets[1].clear();
	_shortcutSupersets[2].clear();

	//Add Alt-F4 as a fake shortcut to prevent Alt-F4 from triggering Alt or F4 key bindings. (e.g load save state 4)
	KeyCombination keyComb;
	keyComb.Key1 = KeyManager::GetKeyCode("Alt");
	keyComb.Key2 = KeyManager::GetKeyCode("F4");
	SetShortcutKey(EmulatorShortcut::Exit, keyComb, 2);
}

void EmuSettings::SetShortcutKey(EmulatorShortcut shortcut, KeyCombination keyCombination, int keySetIndex)
{
	_emulatorKeys[keySetIndex][(uint32_t)shortcut] = keyCombination;

	for(int i = 0; i < 3; i++) {
		for(std::pair<const uint32_t, KeyCombination> &kvp : _emulatorKeys[i]) {
			if(keyCombination.IsSubsetOf(kvp.second)) {
				_shortcutSupersets[keySetIndex][(uint32_t)shortcut].push_back(kvp.second);
			} else if(kvp.second.IsSubsetOf(keyCombination)) {
				_shortcutSupersets[i][kvp.first].push_back(keyCombination);
			}
		}
	}
}

void EmuSettings::SetShortcutKeys(vector<ShortcutKeyInfo> shortcuts)
{
	ClearShortcutKeys();

	for(ShortcutKeyInfo &shortcut : shortcuts) {
		if(_emulatorKeys[0][(uint32_t)shortcut.Shortcut].GetKeys().empty()) {
			SetShortcutKey(shortcut.Shortcut, shortcut.Keys, 0);
		} else {
			SetShortcutKey(shortcut.Shortcut, shortcut.Keys, 1);
		}
	}
}

KeyCombination EmuSettings::GetShortcutKey(EmulatorShortcut shortcut, int keySetIndex)
{
	auto result = _emulatorKeys[keySetIndex].find((int)shortcut);
	if(result != _emulatorKeys[keySetIndex].end()) {
		return result->second;
	}
	return {};
}

vector<KeyCombination> EmuSettings::GetShortcutSupersets(EmulatorShortcut shortcut, int keySetIndex)
{
	return _shortcutSupersets[keySetIndex][(uint32_t)shortcut];
}

OverscanDimensions EmuSettings::GetOverscan()
{
	OverscanDimensions overscan;
	overscan.Left = _video.OverscanLeft;
	overscan.Right = _video.OverscanRight;
	overscan.Top = _video.OverscanTop;
	overscan.Bottom = _video.OverscanBottom;
	return overscan;
}

uint32_t EmuSettings::GetRewindBufferSize()
{
	return _preferences.RewindBufferSize;
}

uint32_t EmuSettings::GetEmulationSpeed()
{
	if(CheckFlag(EmulationFlags::MaximumSpeed)) {
		return 0;
	} else if(CheckFlag(EmulationFlags::Turbo)) {
		return _emulation.TurboSpeed;
	} else if(CheckFlag(EmulationFlags::Rewind)) {
		return _emulation.RewindSpeed;
	} else {
		return _emulation.EmulationSpeed;
	}
}

double EmuSettings::GetAspectRatio(ConsoleRegion region)
{
	switch(_video.AspectRatio) {
		case VideoAspectRatio::NoStretching: return 0.0;
		case VideoAspectRatio::Auto: return region == ConsoleRegion::Pal ? (9440000.0 / 6384411.0) : (128.0 / 105.0);
		case VideoAspectRatio::NTSC: return 128.0 / 105.0;
		case VideoAspectRatio::PAL: return 9440000.0 / 6384411.0;
		case VideoAspectRatio::Standard: return 4.0 / 3.0;
		case VideoAspectRatio::Widescreen: return 16.0 / 9.0;
		case VideoAspectRatio::Custom: return _video.CustomAspectRatio;
	}
	return 0.0;
}

void EmuSettings::SetFlag(EmulationFlags flag)
{
	if((_flags & (int)flag) == 0) {
		_flags |= (int)flag;
	}
}

void EmuSettings::SetFlagState(EmulationFlags flag, bool enabled)
{
	if(enabled) {
		SetFlag(flag);
	} else {
		ClearFlag(flag);
	}
}

void EmuSettings::ClearFlag(EmulationFlags flag)
{
	if((_flags & (int)flag) != 0) {
		_flags &= ~(int)flag;
	}
}

bool EmuSettings::CheckFlag(EmulationFlags flag)
{
	return (_flags & (int)flag) != 0;
}

void EmuSettings::SetDebuggerFlag(DebuggerFlags flag, bool enabled)
{
	if(enabled) {
		if((_debuggerFlags & (int)flag) == 0) {
			_debuggerFlags |= (int)flag;
		}
	} else {
		if((_debuggerFlags & (int)flag) != 0) {
			_debuggerFlags &= ~(int)flag;
		}
	}
}

bool EmuSettings::CheckDebuggerFlag(DebuggerFlags flag)
{
	return (_debuggerFlags & (int)flag) != 0;
}

void EmuSettings::InitializeRam(void* data, uint32_t length)
{
	switch(_emulation.RamPowerOnState) {
		default:
		case RamState::AllZeros: memset(data, 0, length); break;
		case RamState::AllOnes: memset(data, 0xFF, length); break;
		case RamState::Random:
			std::uniform_int_distribution<> dist(0, 255);
			for(uint32_t i = 0; i < length; i++) {
				((uint8_t*)data)[i] = dist(_mt);
			}
			break;
	}
}

int EmuSettings::GetRandomValue(int maxValue)
{
	std::uniform_int_distribution<> dist(0, maxValue);
	return dist(_mt);
}

bool EmuSettings::GetRandomBool()
{ 
	return GetRandomValue(1) == 1; 
}

bool EmuSettings::IsInputEnabled()
{
	return !CheckFlag(EmulationFlags::InBackground) || _preferences.AllowBackgroundInput;
}

double EmuSettings::GetControllerDeadzoneRatio()
{
	switch(_input.ControllerDeadzoneSize) {
		case 0: return 0.5;
		case 1: return 0.75;
		case 2: return 1;
		case 3: return 1.25;
		case 4: return 1.5;
	}
	return 1;
}
