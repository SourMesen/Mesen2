#include "pch.h"
#include <random>
#include "Shared/EmuSettings.h"
#include "Shared/KeyManager.h"
#include "Shared/MessageManager.h"
#include "Shared/Emulator.h"
#include "Shared/DebuggerRequest.h"
#include "Shared/NotificationManager.h"
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

void EmuSettings::CopySettings(EmuSettings& src)
{
	SetVideoConfig(src._video);
	SetAudioConfig(src._audio);
	SetInputConfig(src._input);
	SetEmulationConfig(src._emulation);
	SetPreferences(src._preferences);
	SetAudioPlayerConfig(src._audioPlayer);
	SetDebugConfig(src._debug);
	SetGameConfig(src._game);
	SetSnesConfig(src._snes);
	SetGameboyConfig(src._gameboy);
	SetNesConfig(src._nes);
	SetPcEngineConfig(src._pce);
	SetSmsConfig(src._sms);
	SetGbaConfig(src._gba);
}

void EmuSettings::Serialize(Serializer& s)
{
	//Save/load settings that have an impact on emulation (for movies), netplay, etc.)
	//TODOv2: These should probably not be loaded except for movie playback and netplay clients
	//TODOv2: Desyncs are possible when random state options are turned on
	SV(_video.IntegerFpsMode);
	SV(_emulation.RunAheadFrames);
	SV(_game.DipSwitches);

	switch(_emu->GetConsoleType()) {
		case ConsoleType::Nes:
			SV(_nes.ConsoleType);
			SV(_nes.RamPowerOnState);
			SV(_nes.RandomizeMapperPowerOnState);
			SV(_nes.RandomizeCpuPpuAlignment);
			SV(_nes.DisableOamAddrBug); SV(_nes.DisablePaletteRead); SV(_nes.DisablePpu2004Reads);
			SV(_nes.DisableGameGenieBusConflicts); SV(_nes.DisablePpuReset); SV(_nes.EnableOamDecay);
			SV(_nes.EnablePpu2000ScrollGlitch); SV(_nes.EnablePpu2006ScrollGlitch); SV(_nes.EnablePpuOamRowCorruption);
			SV(_nes.RestrictPpuAccessOnFirstFrame);
			SV(_nes.EnableCpuTestMode);
			SV(_nes.EnableDmcSampleDuplicationGlitch);
			SV(_nes.EnablePpuSpriteEvalBug);
			SV(_nes.PpuExtraScanlinesAfterNmi); SV(_nes.PpuExtraScanlinesBeforeNmi);
			SV(_nes.Region);
			SV(_nes.LightDetectionRadius);
			SV(_nes.Port1.Type); SV(_nes.Port1SubPorts[0].Type); SV(_nes.Port1SubPorts[1].Type); SV(_nes.Port1SubPorts[2].Type); SV(_nes.Port1SubPorts[3].Type);
			SV(_nes.Port2.Type);
			SV(_nes.ExpPort.Type); SV(_nes.ExpPortSubPorts[0].Type); SV(_nes.ExpPortSubPorts[1].Type); SV(_nes.ExpPortSubPorts[2].Type); SV(_nes.ExpPortSubPorts[3].Type);
			break;

		case ConsoleType::Snes:
			SV(_snes.RamPowerOnState);
			SV(_snes.EnableRandomPowerOnState);
			SV(_snes.GsuClockSpeed);
			SV(_snes.PpuExtraScanlinesAfterNmi); SV(_snes.PpuExtraScanlinesBeforeNmi);
			SV(_snes.Region);
			SV(_snes.Port1.Type); SV(_snes.Port1SubPorts[0].Type); SV(_snes.Port1SubPorts[1].Type); SV(_snes.Port1SubPorts[2].Type); SV(_snes.Port1SubPorts[3].Type);
			SV(_snes.Port2.Type); SV(_snes.Port2SubPorts[0].Type); SV(_snes.Port2SubPorts[1].Type); SV(_snes.Port2SubPorts[2].Type); SV(_snes.Port2SubPorts[3].Type);
			SV(_snes.BsxCustomDate);
			SV(_snes.SpcClockSpeedAdjustment);
			
			if(_emu->GetRomInfo().Format == RomFormat::Gb) {
				SV(_gameboy.RamPowerOnState);
				SV(_gameboy.Controller.Type);
				SV(_gameboy.Model);
				SV(_gameboy.UseSgb2);
			}
			break;

		case ConsoleType::Gameboy:
			SV(_gameboy.RamPowerOnState);
			SV(_gameboy.Controller.Type);
			SV(_gameboy.Model);
			SV(_gameboy.UseSgb2);
			break;

		case ConsoleType::PcEngine:
			SV(_pce.RamPowerOnState);
			SV(_pce.EnableRandomPowerOnState);
			SV(_pce.CdRomType);
			SV(_pce.ConsoleType);
			SV(_pce.DisableCdRomSaveRamForHuCardGames);
			SV(_pce.EnableCdRomForHuCardGames);
			SV(_pce.Port1.Type);
			SV(_pce.Port1SubPorts[0].Type);
			SV(_pce.Port1SubPorts[1].Type);
			SV(_pce.Port1SubPorts[2].Type);
			SV(_pce.Port1SubPorts[3].Type);
			SV(_pce.Port1SubPorts[4].Type);
			break;

		case ConsoleType::Sms:
			SV(_sms.RamPowerOnState);
			SV(_sms.Port1.Type);
			SV(_sms.Port2.Type);
			SV(_sms.Region);
			SV(_sms.Revision);
			SV(_sms.EnableFmAudio);
			break;

		case ConsoleType::Gba:
			SV(_gba.RamPowerOnState);
			SV(_gba.OverclockScanlineCount);
			SV(_gba.Controller.Type);
			break;

		case ConsoleType::Ws:
			//TODOWS
			break;

		default:
			throw std::runtime_error("unsupported console type");
	}
}

uint32_t EmuSettings::GetVersion()
{
	//Version 2.1.1
	uint16_t major = 2;
	uint8_t minor = 1;
	uint8_t revision = 1;
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
	_audio = config;
	ProcessString(_audioDevice, &_audio.AudioDevice);
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

void EmuSettings::SetGbaConfig(GbaConfig& config)
{
	_gba = config;
}

GbaConfig& EmuSettings::GetGbaConfig()
{
	return _gba;
}

void EmuSettings::SetPcEngineConfig(PcEngineConfig& config)
{
	_pce = config;
}

PcEngineConfig& EmuSettings::GetPcEngineConfig()
{
	return _pce;
}

void EmuSettings::SetSmsConfig(SmsConfig& config)
{
	_sms = config;
}

SmsConfig& EmuSettings::GetSmsConfig()
{
	return _sms;
}

void EmuSettings::SetCvConfig(CvConfig& config)
{
	_cv = config;
}

CvConfig& EmuSettings::GetCvConfig()
{
	return _cv;
}

void EmuSettings::SetWsConfig(WsConfig& config)
{
	_ws = config;
}

WsConfig& EmuSettings::GetWsConfig()
{
	return _ws;
}

void EmuSettings::SetGameConfig(GameConfig& config)
{
	_game = config;
}

GameConfig& EmuSettings::GetGameConfig()
{
	return _game;
}

void EmuSettings::SetPreferences(PreferencesConfig& config)
{
	MessageManager::SetOptions(!config.DisableOsd, CheckFlag(EmulationFlags::OutputToStdout));

	_preferences = config;

	ProcessString(_saveFolder, &_preferences.SaveFolderOverride);
	ProcessString(_saveStateFolder, &_preferences.SaveStateFolderOverride);
	ProcessString(_screenshotFolder, &_preferences.ScreenshotFolderOverride);

	FolderUtilities::SetFolderOverrides(
		_saveFolder,
		_saveStateFolder,
		_screenshotFolder,
		""
	);
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
	
	DebuggerRequest req = _emu->GetDebugger(false);
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
	RomFormat romFormat = _emu->GetRomInfo().Format;
	switch(romFormat) {
		case RomFormat::Spc:
		case RomFormat::Gbs:
		case RomFormat::Nsf:
		case RomFormat::PceHes:
			//No overscan for music players
			return OverscanDimensions {};

		case RomFormat::Gb:
			if(_emu->GetConsoleType() == ConsoleType::Snes && _emu->GetSettings()->GetGameboyConfig().HideSgbBorders) {
				//Override overscan dimensions to hide SGB borders
				OverscanDimensions overscan = {};
				overscan.Top =  46;
				overscan.Bottom = 49;
				overscan.Left = 48;
				overscan.Right = 48;
				return overscan;
			}
	}

	if(_game.OverrideOverscan) {
		return _game.Overscan;
	}

	switch(_emu->GetConsoleType()) {
		case ConsoleType::Snes: return _snes.Overscan;
		case ConsoleType::Nes: return _emu->GetRegion() == ConsoleRegion::Ntsc ? _nes.NtscOverscan : _nes.PalOverscan;
		case ConsoleType::PcEngine: return _pce.Overscan;
		case ConsoleType::Sms:
			if(romFormat == RomFormat::ColecoVision) {
				return { 0, 0, 24, 24 };
			} else if(romFormat == RomFormat::GameGear) {
				return _sms.GameGearOverscan;
			} else {
				return  _emu->GetRegion() == ConsoleRegion::Ntsc ? _sms.NtscOverscan : _sms.PalOverscan;
			}

		case ConsoleType::Gameboy:
		case ConsoleType::Gba:
		case ConsoleType::Ws:
			break;
	}

	return OverscanDimensions {};
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
		case VideoAspectRatio::Auto:
			if(_emu->GetConsoleType() == ConsoleType::Gameboy || _emu->GetConsoleType() == ConsoleType::Gba || _emu->GetConsoleType() == ConsoleType::Ws) {
				//GB/GBA/WS shouldn't use NTSC/PAL aspect ratio when in auto mode
				return screenAspectRatio;
			} else if(_emu->GetRomInfo().Format == RomFormat::GameGear) {
				//GG has a 6:5 PAR
				return screenAspectRatio * (6.0 / 5.0);
			}
			return screenAspectRatio * ((region == ConsoleRegion::Pal || region == ConsoleRegion::Dendy) ? (11.0 / 8.0) : (8.0 / 7.0));

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

	DebuggerRequest req = _emu->GetDebugger(false);
	Debugger* dbg = req.GetDebugger();
	if(dbg) {
		dbg->ProcessConfigChange();
	}
}

bool EmuSettings::CheckDebuggerFlag(DebuggerFlags flag)
{
	return (_debuggerFlags & (uint64_t)flag) != 0;
}

bool EmuSettings::HasRandomPowerOnState(ConsoleType consoleType)
{
	switch(consoleType) {
		case ConsoleType::Snes: return _snes.RamPowerOnState == RamState::Random || _snes.EnableRandomPowerOnState;
		case ConsoleType::Gameboy: return _gameboy.RamPowerOnState == RamState::Random;
		case ConsoleType::Nes: return _nes.RamPowerOnState == RamState::Random || _nes.RandomizeCpuPpuAlignment || _nes.RandomizeMapperPowerOnState;
		case ConsoleType::PcEngine: return _pce.RamPowerOnState == RamState::Random || _pce.EnableRandomPowerOnState;
		case ConsoleType::Sms: return _sms.RamPowerOnState == RamState::Random;
		case ConsoleType::Gba: return _gba.RamPowerOnState == RamState::Random;
	}

	return false;
}

void EmuSettings::InitializeRam(RamState state, void* data, uint32_t length)
{
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
