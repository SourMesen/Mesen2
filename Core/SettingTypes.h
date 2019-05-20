#pragma once
#include "stdafx.h"
#include <algorithm>

enum class EmulationFlags
{
	Turbo = 1,
	Rewind = 2,
	MaximumSpeed = 4
};

enum class ScaleFilterType
{
	xBRZ = 0,
	HQX = 1,
	Scale2x = 2,
	_2xSai = 3,
	Super2xSai = 4,
	SuperEagle = 5,
	Prescale = 6,
};

enum class VideoFilterType
{
	None = 0,
	NTSC,
	xBRZ2x,
	xBRZ3x,
	xBRZ4x,
	xBRZ5x,
	xBRZ6x,
	HQ2x,
	HQ3x,
	HQ4x,
	Scale2x,
	Scale3x,
	Scale4x,
	_2xSai,
	Super2xSai,
	SuperEagle,
	Prescale2x,
	Prescale3x,
	Prescale4x,
	Prescale6x,
	Prescale8x,
	Prescale10x
};

enum class VideoResizeFilter
{
	NearestNeighbor = 0,
	Bilinear = 1
};

enum class VideoAspectRatio
{
	NoStretching = 0,
	Auto = 1,
	NTSC = 2,
	PAL = 3,
	Standard = 4,
	Widescreen = 5,
	Custom = 6
};

struct VideoConfig
{
	double VideoScale = 2;
	double CustomAspectRatio = 1.0;
	VideoFilterType VideoFilter = VideoFilterType::None;
	VideoAspectRatio AspectRatio = VideoAspectRatio::NoStretching;
	bool UseBilinearInterpolation = false;
	bool VerticalSync = false;
	bool IntegerFpsMode = false;

	double Brightness = 0;
	double Contrast = 0;
	double Hue = 0;
	double Saturation = 0;
	double ScanlineIntensity = 0;

	double NtscArtifacts = 0;
	double NtscBleed = 0;
	double NtscFringing = 0;
	double NtscGamma = 0;
	double NtscResolution = 0;
	double NtscSharpness = 0;
	bool NtscMergeFields = false;

	uint32_t OverscanLeft = 0;
	uint32_t OverscanRight = 0;
	uint32_t OverscanTop = 0;
	uint32_t OverscanBottom = 0;

	bool FullscreenForceIntegerScale = false;
	bool UseExclusiveFullscreen = false;
	uint32_t ExclusiveFullscreenRefreshRate = 60;
};

struct AudioConfig
{
	const char* AudioDevice = nullptr;
	bool EnableAudio = true;
	bool DisableDynamicSampleRate = false;

	uint32_t MasterVolume = 100;
	uint32_t SampleRate = 48000;
	uint32_t AudioLatency = 60;

	bool MuteSoundInBackground = false;
	bool ReduceSoundInBackground = true;
	bool ReduceSoundInFastForward = false;
	uint32_t VolumeReduction = 75;

	bool EnableEqualizer = false;
	double Band1Gain = 0;
	double Band2Gain = 0;
	double Band3Gain = 0;
	double Band4Gain = 0;
	double Band5Gain = 0;
	double Band6Gain = 0;
	double Band7Gain = 0;
	double Band8Gain = 0;
	double Band9Gain = 0;
	double Band10Gain = 0;
	double Band11Gain = 0;
	double Band12Gain = 0;
	double Band13Gain = 0;
	double Band14Gain = 0;
	double Band15Gain = 0;
	double Band16Gain = 0;
	double Band17Gain = 0;
	double Band18Gain = 0;
	double Band19Gain = 0;
	double Band20Gain = 0;
};

enum class ControllerType
{
	None = 0,
	SnesController = 1,
	SnesMouse = 2,
	SuperScope = 3
};

struct KeyMapping
{
	uint32_t A = 0;
	uint32_t B = 0;
	uint32_t X = 0;
	uint32_t Y = 0;
	uint32_t L = 0;
	uint32_t R = 0;
	uint32_t Up = 0;
	uint32_t Down = 0;
	uint32_t Left = 0;
	uint32_t Right = 0;
	uint32_t Start = 0;
	uint32_t Select = 0;

	uint32_t TurboA = 0;
	uint32_t TurboB = 0;
	uint32_t TurboX = 0;
	uint32_t TurboY = 0;
	uint32_t TurboL = 0;
	uint32_t TurboR = 0;
	uint32_t TurboSelect = 0;
	uint32_t TurboStart = 0;

	bool HasKeySet()
	{
		if(A || B || X || Y || L || R || Up || Down || Left || Right || Start || Select || TurboA || TurboB || TurboX || TurboY || TurboL || TurboR || TurboStart || TurboSelect) {
			return true;
		}
		return false;
	}

private:
	bool HasKeyBinding(uint32_t* buttons, uint32_t count)
	{
		for(uint32_t i = 0; i < count; i++) {
			if(buttons[i] != 0) {
				return true;
			}
		}
		return false;
	}
};

struct KeyMappingSet
{
	KeyMapping Mapping1;
	KeyMapping Mapping2;
	KeyMapping Mapping3;
	KeyMapping Mapping4;
	uint32_t TurboSpeed = 0;

	vector<KeyMapping> GetKeyMappingArray()
	{
		vector<KeyMapping> keyMappings;
		if(Mapping1.HasKeySet()) {
			keyMappings.push_back(Mapping1);
		}
		if(Mapping2.HasKeySet()) {
			keyMappings.push_back(Mapping2);
		}
		if(Mapping3.HasKeySet()) {
			keyMappings.push_back(Mapping3);
		}
		if(Mapping4.HasKeySet()) {
			keyMappings.push_back(Mapping4);
		}
		return keyMappings;
	}
};

struct ControllerConfig
{
	KeyMappingSet Keys;
	ControllerType Type = ControllerType::SnesController;
};

struct InputConfig
{
	ControllerConfig Controllers[4];
	uint32_t ControllerDeadzoneSize = 2;
	uint32_t MouseSensitivity = 1;
};

enum class RamState
{
	AllZeros = 0,
	AllOnes = 1,
	Random = 2
};

enum class ConsoleRegion
{
	Auto = 0,
	Ntsc = 1,
	Pal = 2
};

struct EmulationConfig
{
	uint32_t EmulationSpeed = 100;
	uint32_t TurboSpeed = 300;
	uint32_t RewindSpeed = 100;

	ConsoleRegion Region = ConsoleRegion::Auto;

	bool AllowInvalidInput = false;
	bool EnableRandomPowerOnState = false;

	uint32_t PpuExtraScanlinesBeforeNmi = 0;
	uint32_t PpuExtraScanlinesAfterNmi = 0;

	RamState RamPowerOnState = RamState::AllZeros;
};

struct PreferencesConfig
{
	bool ShowFps = false;
	bool ShowFrameCounter = false;
	bool ShowGameTimer = false;
	bool ShowDebugInfo = false;
	bool DisableOsd = false;

	uint32_t RewindBufferSize = 600;

	const char* SaveFolderOverride = nullptr;
	const char* SaveStateFolderOverride = nullptr;
	const char* ScreenshotFolderOverride = nullptr;
};

struct OverscanDimensions
{
	uint32_t Left = 0;
	uint32_t Right = 0;
	uint32_t Top = 0;
	uint32_t Bottom = 0;
};

struct FrameInfo
{
	uint32_t Width;
	uint32_t Height;
};

struct ScreenSize
{
	int32_t Width;
	int32_t Height;
	double Scale;
};

enum class EmulatorShortcut
{
	FastForward,
	Rewind,
	RewindTenSecs,
	RewindOneMin,

	MoveToNextStateSlot,
	MoveToPreviousStateSlot,
	SaveState,
	LoadState,

	ToggleAudio,
	ToggleFastForward,
	ToggleRewind,

	RunSingleFrame,

	// Everything below this is handled UI-side
	TakeScreenshot,

	IncreaseSpeed,
	DecreaseSpeed,
	MaxSpeed,

	Pause,
	Reset,
	PowerCycle,
	PowerOff,
	Exit,

	SetScale1x,
	SetScale2x,
	SetScale3x,
	SetScale4x,
	SetScale5x,
	SetScale6x,
	ToggleFullscreen,
	ToggleFps,
	ToggleGameTimer,
	ToggleFrameCounter,
	ToggleOsd,
	ToggleAlwaysOnTop,
	ToggleDebugInfo,

	SaveStateSlot1,
	SaveStateSlot2,
	SaveStateSlot3,
	SaveStateSlot4,
	SaveStateSlot5,
	SaveStateSlot6,
	SaveStateSlot7,
	SaveStateSlot8,
	SaveStateSlot9,
	SaveStateSlot10,
	SaveStateToFile,

	LoadStateSlot1,
	LoadStateSlot2,
	LoadStateSlot3,
	LoadStateSlot4,
	LoadStateSlot5,
	LoadStateSlot6,
	LoadStateSlot7,
	LoadStateSlot8,
	LoadStateSlot9,
	LoadStateSlot10,
	LoadStateSlotAuto,
	LoadStateFromFile,

	OpenFile,
	ShortcutCount
};

struct KeyCombination
{
	uint32_t Key1 = 0;
	uint32_t Key2 = 0;
	uint32_t Key3 = 0;

	vector<uint32_t> GetKeys()
	{
		vector<uint32_t> result;
		if(Key1) {
			result.push_back(Key1);
		}
		if(Key2) {
			result.push_back(Key2);
		}
		if(Key3) {
			result.push_back(Key3);
		}
		return result;
	}

	bool IsSubsetOf(KeyCombination keyCombination)
	{
		vector<uint32_t> myKeys = GetKeys();
		vector<uint32_t> otherKeys = keyCombination.GetKeys();

		if(otherKeys.size() > myKeys.size()) {
			for(size_t i = 0; i < myKeys.size(); i++) {
				if(std::find(otherKeys.begin(), otherKeys.end(), myKeys[i]) == otherKeys.end()) {
					//Current key combination contains a key not found in the other combination, so it's not a subset
					return false;
				}
			}
			return true;
		}
		return false;
	}
};

struct ShortcutKeyInfo
{
	EmulatorShortcut Shortcut;
	KeyCombination Keys;
};

enum class DebuggerFlags : uint32_t
{
	BreakOnBrk = 0x01,
	BreakOnCop = 0x02,
	BreakOnWdm = 0x04,
	BreakOnStp = 0x08,
	BreakOnUninitRead = 0x10,

	ShowVerifiedData = 0x100,
	DisassembleVerifiedData = 0x200,
	
	ShowUnidentifiedData = 0x400,
	DisassembleUnidentifiedData = 0x800,

	SpcDebuggerEnabled = 0x40000000,
	CpuDebuggerEnabled = 0x80000000
};
