#include "stdafx.h"
#include <random>
#include "EmuSettings.h"
#include "KeyManager.h"
#include "MessageManager.h"
#include "Emulator.h"
#include "NotificationManager.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/Serializer.h"

EmuSettings::EmuSettings(Emulator* emu)
{
	_emu = emu;
	_flags = 0;
	_debuggerFlags = 0;

	std::random_device rd;
	_mt = std::mt19937(rd());
}

void EmuSettings::Serialize(Serializer& s)
{
	//Save/load settings that have an impact on emulation (for movies), netplay), etc.)
	SV(_video.IntegerFpsMode);
	SV(_emulation.RunAheadFrames);

	switch(_emu->GetConsoleType()) {
		case ConsoleType::Nes:
			SV(_nes.ConsoleType); SV(_nes.DipSwitches);
			SV(_nes.DisableOamAddrBug); SV(_nes.DisablePaletteRead); SV(_nes.DisablePpu2004Reads);
			SV(_nes.DisableGameGenieBusConflicts); SV(_nes.DisablePpuReset); SV(_nes.EnableOamDecay);
			SV(_nes.EnablePpu2000ScrollGlitch); SV(_nes.EnablePpu2006ScrollGlitch); SV(_nes.EnablePpuOamRowCorruption);
			SV(_nes.PpuExtraScanlinesAfterNmi); SV(_nes.PpuExtraScanlinesBeforeNmi);
			SV(_nes.Region);
			SV(_nes.LightDetectionRadius);
			SV(_nes.Port1.Type); SV(_nes.Port1SubPorts[0].Type); SV(_nes.Port1SubPorts[1].Type); SV(_nes.Port1SubPorts[2].Type); SV(_nes.Port1SubPorts[3].Type);
			SV(_nes.Port2.Type);
			SV(_nes.ExpPort.Type); SV(_nes.ExpPortSubPorts[0].Type); SV(_nes.ExpPortSubPorts[1].Type); SV(_nes.ExpPortSubPorts[2].Type); SV(_nes.ExpPortSubPorts[3].Type);
			break;

		case ConsoleType::Snes:
			SV(_snes.GsuClockSpeed);
			SV(_snes.PpuExtraScanlinesAfterNmi); SV(_snes.PpuExtraScanlinesBeforeNmi);
			SV(_snes.Region);
			SV(_snes.Port1.Type); SV(_snes.Port1SubPorts[0].Type); SV(_snes.Port1SubPorts[1].Type); SV(_snes.Port1SubPorts[2].Type); SV(_snes.Port1SubPorts[3].Type);
			SV(_snes.Port2.Type); SV(_snes.Port2SubPorts[0].Type); SV(_snes.Port2SubPorts[1].Type); SV(_snes.Port2SubPorts[2].Type); SV(_snes.Port2SubPorts[3].Type);
			SV(_snes.BsxCustomDate);
			break;

		case ConsoleType::Gameboy:
		case ConsoleType::GameboyColor:
			SV(_gameboy.Controller.Type);
			SV(_gameboy.Model);
			SV(_gameboy.UseSgb2);
			break;


		case ConsoleType::PcEngine:
			break;

		default:
			throw std::runtime_error("unsupport console type");
	}
}

uint32_t EmuSettings::GetVersion()
{
	//Version 2.0.0
	uint16_t major = 2;
	uint8_t minor = 0;
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

void EmuSettings::SetVideoConfig(VideoConfig& config)
{
	_video = config;
}

VideoConfig& EmuSettings::GetVideoConfig()
{
	return _video;
}

void EmuSettings::SetAudioConfig(AudioConfig& config)
{
	ProcessString(_audioDevice, &config.AudioDevice);

	_audio = config;
}

AudioConfig& EmuSettings::GetAudioConfig()
{
	return _audio;
}

void EmuSettings::SetInputConfig(InputConfig& config)
{
	_input = config;
}

InputConfig& EmuSettings::GetInputConfig()
{
	return _input;
}

void EmuSettings::SetEmulationConfig(EmulationConfig& config)
{
	_emulation = config;
}

EmulationConfig& EmuSettings::GetEmulationConfig()
{
	return _emulation;
}

void EmuSettings::SetSnesConfig(SnesConfig& config)
{
	_snes = config;
}

SnesConfig& EmuSettings::GetSnesConfig()
{
	return _snes;
}

void EmuSettings::SetNesConfig(NesConfig& config)
{
	_nes = config;
}

NesConfig& EmuSettings::GetNesConfig()
{
	return _nes;
}

void EmuSettings::SetGameboyConfig(GameboyConfig& config)
{
	_gameboy = config;
}

GameboyConfig& EmuSettings::GetGameboyConfig()
{
	return _gameboy;
}

void EmuSettings::SetPcEngineConfig(PcEngineConfig& config)
{
	_pce = config;
}

PcEngineConfig& EmuSettings::GetPcEngineConfig()
{
	return _pce;
}

void EmuSettings::SetPreferences(PreferencesConfig& config)
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

PreferencesConfig& EmuSettings::GetPreferences()
{
	return _preferences;
}

void EmuSettings::SetAudioPlayerConfig(AudioPlayerConfig& config)
{
	_audioPlayer = config;
}

AudioPlayerConfig& EmuSettings::GetAudioPlayerConfig()
{
	return _audioPlayer;
}

void EmuSettings::SetDebugConfig(DebugConfig& config)
{
	_debug = config;
	
	Emulator::DebuggerRequest req = _emu->GetDebugger(false);
	Debugger* dbg = req.GetDebugger();
	if(dbg) {
		dbg->ProcessConfigChange();
	}
}

DebugConfig& EmuSettings::GetDebugConfig()
{
	return _debug;
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
	keyComb.Key1 = KeyManager::GetKeyCode("Left Alt");
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
	auto lock = _updateShortcutsLock.AcquireSafe();
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
	auto lock = _updateShortcutsLock.AcquireSafe();
	auto result = _emulatorKeys[keySetIndex].find((int)shortcut);
	if(result != _emulatorKeys[keySetIndex].end()) {
		return result->second;
	}
	return {};
}

vector<KeyCombination> EmuSettings::GetShortcutSupersets(EmulatorShortcut shortcut, int keySetIndex)
{
	auto lock = _updateShortcutsLock.AcquireSafe();
	return _shortcutSupersets[keySetIndex][(uint32_t)shortcut];
}

OverscanDimensions EmuSettings::GetOverscan()
{
	OverscanDimensions overscan = {};

	switch(_emu->GetRomInfo().Format) {
		case RomFormat::Spc:
		case RomFormat::Gbs:
		case RomFormat::Nsf:
			//No overscan for music players
			return overscan;
	}

	switch(_emu->GetConsoleType()) {
		case ConsoleType::Snes: 
			overscan.Left = _snes.OverscanLeft;
			overscan.Right = _snes.OverscanRight;
			overscan.Top = _snes.OverscanTop;
			overscan.Bottom = _snes.OverscanBottom;
			break;

		case ConsoleType::Nes:
			overscan.Left = _nes.OverscanLeft;
			overscan.Right = _nes.OverscanRight;
			overscan.Top = _nes.OverscanTop;
			overscan.Bottom = _nes.OverscanBottom;
			break;

		case ConsoleType::Gameboy:
		case ConsoleType::GameboyColor:
			break;
	}

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

double EmuSettings::GetAspectRatio(ConsoleRegion region, FrameInfo baseFrameSize)
{
	double screenAspectRatio = (double)baseFrameSize.Width / baseFrameSize.Height;

	switch(_video.AspectRatio) {
		case VideoAspectRatio::NoStretching: return screenAspectRatio;
		
		//For auto, ntsc and pal, these are PAR ratios, so multiply them with the base screen's aspect ratio to get the expected screen aspect ratio
		case VideoAspectRatio::Auto: return screenAspectRatio * ((region == ConsoleRegion::Pal || region == ConsoleRegion::Dendy) ? (11.0 / 8.0) : (8.0 / 7.0));
		case VideoAspectRatio::NTSC: return screenAspectRatio * 8.0 / 7.0;
		case VideoAspectRatio::PAL: return screenAspectRatio * 11.0 / 8.0;

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
		if((_debuggerFlags & (uint64_t)flag) == 0) {
			_debuggerFlags |= (uint64_t)flag;
		}
	} else {
		if((_debuggerFlags & (uint64_t)flag) != 0) {
			_debuggerFlags &= ~(uint64_t)flag;
		}
	}

	Emulator::DebuggerRequest req = _emu->GetDebugger(false);
	Debugger* dbg = req.GetDebugger();
	if(dbg) {
		dbg->ProcessConfigChange();
	}
}

bool EmuSettings::CheckDebuggerFlag(DebuggerFlags flag)
{
	return (_debuggerFlags & (uint64_t)flag) != 0;
}

void EmuSettings::InitializeRam(void* data, uint32_t length)
{
	RamState state;
	switch(_emu->GetConsoleType()) {
		default:
		case ConsoleType::Snes: state = _snes.RamPowerOnState; break;
		case ConsoleType::Nes: state = _nes.RamPowerOnState; break;
		
		case ConsoleType::Gameboy:
		case ConsoleType::GameboyColor:
			state = _gameboy.RamPowerOnState;
			break;

		case ConsoleType::PcEngine: state = _pce.RamPowerOnState; break;
	}

	switch(state) {
		default:
		case RamState::AllZeros: memset(data, 0, length); break;
		case RamState::AllOnes: memset(data, 0xFF, length); break;
		case RamState::Random:
			std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());
			uint32_t i = 0;
			while(i < length) {
				uint64_t randomData = dist(_mt);
				for(int j = 0; j < 8 && i < length; j++) {
					((uint8_t*)data)[i] = (uint8_t)(randomData >> (8*j));
					i++;
				}
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
