#pragma once
#include "stdafx.h"
#include <algorithm>

enum class EmulationFlags
{
	Turbo = 0x01,
	Rewind = 0x02,
	TurboOrRewind = 0x03,
	MaximumSpeed = 0x04,
	InBackground = 0x08,
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

	bool FullscreenForceIntegerScale = false;
	bool UseExclusiveFullscreen = false;
	uint32_t ExclusiveFullscreenRefreshRate = 60;
	uint32_t FullscreenResWidth = 0;
	uint32_t FullscreenResHeight = 0;
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

	bool ReverbEnabled;
	uint32_t ReverbStrength;
	uint32_t ReverbDelay;

	bool CrossFeedEnabled;
	uint32_t CrossFeedRatio;

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

//Update ControllerTypeNames when changing this
enum class ControllerType
{
	None = 0,
	SnesController = 1,
	SnesMouse = 2,
	SuperScope = 3,
	Multitap = 4,
	NesController = 5
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
	
	uint32_t Microphone = 0;

	uint32_t CustomKeys[100];

	bool HasKeySet()
	{
		if(A || B || X || Y || L || R || Up || Down || Left || Right || Start || Select || TurboA || TurboB || TurboX || TurboY || TurboL || TurboR || TurboStart || TurboSelect) {
			return true;
		}
		for(uint32_t i = 0; i < 100; i++) {
			if(CustomKeys[i] != 0) {
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

enum class InputDisplayPosition
{
	TopLeft = 0,
	TopRight = 1,
	BottomLeft = 2,
	BottomRight = 3
};

struct InputConfig
{
	uint32_t ControllerDeadzoneSize = 2;
	uint32_t MouseSensitivity = 1;

	InputDisplayPosition DisplayInputPosition = InputDisplayPosition::TopLeft;
	bool DisplayInputPort[5] = { false, false, false, false, false};
	bool DisplayInputHorizontally = true;
};

enum class RamState
{
	Random = 0,
	AllZeros = 1,
	AllOnes = 2,
};

enum class ConsoleRegion
{
	Auto = 0,
	Ntsc = 1,
	Pal = 2
};

enum class ConsoleType
{
	Snes = 0,
	Gameboy = 1,
	GameboyColor = 2,
	Nes = 3
};

enum class GameboyModel
{
	Auto = 0,
	Gameboy = 1,
	GameboyColor = 2,
	SuperGameboy = 3
};

struct EmulationConfig
{
	uint32_t EmulationSpeed = 100;
	uint32_t TurboSpeed = 300;
	uint32_t RewindSpeed = 100;

	ConsoleRegion Region = ConsoleRegion::Auto;

	uint32_t RunAheadFrames = 0;
};

struct GameboyConfig
{
	GameboyModel Model = GameboyModel::Auto;
	bool UseSgb2 = true;

	bool BlendFrames = true;
	bool GbcAdjustColors = true;

	RamState RamPowerOnState = RamState::Random;
	
	uint32_t BgColors[4] = { 0xFFFFFF, 0xB0B0B0, 0x686868, 0x000000 };
	uint32_t Obj0Colors[4] = { 0xFFFFFF, 0xB0B0B0, 0x686868, 0x000000 };
	uint32_t Obj1Colors[4] = { 0xFFFFFF, 0xB0B0B0, 0x686868, 0x000000 };

	uint32_t Square1Vol = 100;
	uint32_t Square2Vol = 100;
	uint32_t NoiseVol = 100;
	uint32_t WaveVol = 100;
};

struct SnesConfig
{
	ControllerConfig Controllers[5];

	bool BlendHighResolutionModes = false;
	bool HideBgLayer0 = false;
	bool HideBgLayer1 = false;
	bool HideBgLayer2 = false;
	bool HideBgLayer3 = false;
	bool HideSprites = false;
	bool DisableFrameSkipping = false;

	uint32_t OverscanLeft = 0;
	uint32_t OverscanRight = 0;
	uint32_t OverscanTop = 7;
	uint32_t OverscanBottom = 8;

	bool EnableCubicInterpolation = true;

	bool EnableRandomPowerOnState = false;
	bool EnableStrictBoardMappings = false;
	RamState RamPowerOnState = RamState::Random;

	uint32_t PpuExtraScanlinesBeforeNmi = 0;
	uint32_t PpuExtraScanlinesAfterNmi = 0;
	uint32_t GsuClockSpeed = 100;

	int64_t BsxCustomDate = -1;
};

enum class StereoFilterType
{
	None = 0,
	Delay = 1,
	Panning = 2,
	CombFilter = 3,
};

enum class VsDualOutputOption
{
	Both = 0,
	MasterOnly = 1,
	SlaveOnly = 2
};

struct NesConfig
{
	ControllerConfig Controllers[5];

	bool EnableHdPacks = true;
	bool DisableGameDatabase = false;
	bool FdsAutoLoadDisk = true;
	bool FdsFastForwardOnLoad = false;
	bool FdsAutoInsertDisk = false;
	VsDualOutputOption VsDualVideoOutput = VsDualOutputOption::Both;
	VsDualOutputOption VsDualAudioOutput = VsDualOutputOption::Both;
	bool NsfMoveToNextTrackAfterTime = true;
	uint32_t NsfMoveToNextTrackTime = 120;
	bool NsfAutoDetectSilence = true;
	uint32_t NsfAutoDetectSilenceDelay = 3000;

	bool SpritesEnabled = true;
	bool BackgroundEnabled = true;
	bool ForceBackgroundFirstColumn = false;
	bool ForceSpritesFirstColumn = false;
	bool RemoveSpriteLimit = false;
	bool AdaptiveSpriteLimit = false;
	
	bool UseCustomVsPalette = false;
	
	uint32_t OverscanLeft = 0;
	uint32_t OverscanRight = 0;
	uint32_t OverscanTop = 0;
	uint32_t OverscanBottom = 0;
	
	bool UseNes101Hvc101Behavior = false;
	bool DisablePpuReset = false;
	bool AllowInvalidInput = false;

	bool EnableOamDecay = false;
	bool EnablePpuOamRowCorruption = false;
	bool DisableOamAddrBug = false;
	bool DisablePaletteRead = false;
	bool DisablePpu2004Reads = false;
	bool EnablePpu2000ScrollGlitch = false;
	bool EnablePpu2006ScrollGlitch = false;

	bool RandomizeMapperPowerOnState = false;
	bool RandomizeCpuPpuAlignment = false;
	RamState RamPowerOnState = RamState::Random;

	uint32_t PpuExtraScanlinesBeforeNmi = 0;
	uint32_t PpuExtraScanlinesAfterNmi = 0;

	bool DisableNoiseModeFlag = false;
	bool ReduceDmcPopping = false;
	bool SilenceTriangleHighFreq = false;
	bool SwapDutyCycles = false;

	bool BreakOnCrash = false;
	uint32_t DipSwitches = 0;

	int32_t InputScanline = 241;

	bool IsFullColorPalette = false;
	uint32_t UserPalette[512] = { };

	uint32_t ChannelVolumes[11];
	uint32_t ChannelPanning[11];

	StereoFilterType StereoFilter;
	int32_t StereoDelay;
	int32_t StereoPanningAngle;
	int32_t StereoCombFilterDelay;
	int32_t StereoCombFilterStrength;
};

struct PreferencesConfig
{
	bool ShowFps = false;
	bool ShowFrameCounter = false;
	bool ShowGameTimer = false;
	bool ShowDebugInfo = false;
	bool DisableOsd = false;
	bool AllowBackgroundInput = false;
	bool PauseOnMovieEnd = false;
	bool DisableGameSelectionScreen = false;

	uint32_t RewindBufferSize = 30;

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

	SelectSaveSlot1,
	SelectSaveSlot2,
	SelectSaveSlot3,
	SelectSaveSlot4,
	SelectSaveSlot5,
	SelectSaveSlot6,
	SelectSaveSlot7,
	SelectSaveSlot8,
	SelectSaveSlot9,
	SelectSaveSlot10,
	MoveToNextStateSlot,
	MoveToPreviousStateSlot,
	SaveState,
	LoadState,

	ToggleCheats,
	ToggleFastForward,
	ToggleRewind,

	RunSingleFrame,

	// Everything below this is handled UI-side
	TakeScreenshot,

	ToggleRecordVideo,
	ToggleRecordAudio,
	ToggleRecordMovie,

	IncreaseSpeed,
	DecreaseSpeed,
	MaxSpeed,

	Pause,
	Reset,
	PowerCycle,
	ReloadRom,
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
	
	ToggleAudio,
	IncreaseVolume,
	DecreaseVolume,

	ToggleBgLayer0,
	ToggleBgLayer1,
	ToggleBgLayer2,
	ToggleBgLayer3,
	ToggleSprites,
	EnableAllLayers,

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
	SaveStateDialog,

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
	LoadStateDialog,

	OpenFile,
	LoadRandomGame,
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

enum class DebuggerFlags : uint64_t
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
	
	UseAltSpcOpNames = 0x1000,
	UseLowerCaseDisassembly = 0x2000,
	
	AutoResetCdl = 0x4000,

	GbBreakOnInvalidOamAccess = 0x10000,
	GbBreakOnInvalidVramAccess = 0x20000,
	GbBreakOnDisableLcdOutsideVblank = 0x40000,
	GbBreakOnInvalidOpCode = 0x80000,
	GbBreakOnNopLoad = 0x100000,
	GbBreakOnOamCorruption = 0x200000,

	NesDebuggerEnabled = 0x01000000,
	GbDebuggerEnabled = 0x02000000,
	Cx4DebuggerEnabled = 0x04000000,
	NecDspDebuggerEnabled = 0x08000000,
	GsuDebuggerEnabled = 0x10000000,
	Sa1DebuggerEnabled = 0x20000000,
	SpcDebuggerEnabled = 0x40000000,
	CpuDebuggerEnabled = 0x80000000,

	NesBreakOnDecayedOamRead = 0x100000000,
	NesBreakOnPpu2006ScrollGlitch = 0x200000000,
};
