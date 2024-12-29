#pragma once
#include "pch.h"
#include <algorithm>

enum class EmulationFlags
{
	Turbo = 0x01,
	Rewind = 0x02,
	TurboOrRewind = 0x03,
	MaximumSpeed = 0x04,
	InBackground = 0x08,
	ConsoleMode = 0x10,
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
	LcdGrid = 7,
};

enum class VideoFilterType
{
	None = 0,
	NtscBlargg,
	NtscBisqwit,
	LcdGrid,
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

enum class NtscBisqwitFilterScale
{
	_2x,
	_4x,
	_8x,
};

struct VideoConfig
{
	double CustomAspectRatio = 1.0;
	VideoFilterType VideoFilter = VideoFilterType::None;
	VideoAspectRatio AspectRatio = VideoAspectRatio::NoStretching;
	bool UseBilinearInterpolation = false;
	bool UseSrgbTextureFormat = false;
	bool VerticalSync = false;
	bool IntegerFpsMode = false;

	double Brightness = 0;
	double Contrast = 0;
	double Hue = 0;
	double Saturation = 0;
	double ScanlineIntensity = 0;

	double LcdGridTopLeftBrightness = 1.0;
	double LcdGridTopRightBrightness = 1.0;
	double LcdGridBottomLeftBrightness = 1.0;
	double LcdGridBottomRightBrightness = 1.0;

	double NtscArtifacts = 0;
	double NtscBleed = 0;
	double NtscFringing = 0;
	double NtscGamma = 0;
	double NtscResolution = 0;
	double NtscSharpness = 0;
	bool NtscMergeFields = false;

	NtscBisqwitFilterScale NtscScale = NtscBisqwitFilterScale::_2x;
	double NtscYFilterLength = 1.0;
	double NtscIFilterLength = 1.0;
	double NtscQFilterLength = 1.0;

	bool FullscreenForceIntegerScale = false;
	bool UseExclusiveFullscreen = false;
	uint32_t ExclusiveFullscreenRefreshRateNtsc = 60;
	uint32_t ExclusiveFullscreenRefreshRatePal = 50;
	uint32_t FullscreenResWidth = 0;
	uint32_t FullscreenResHeight = 0;

	uint32_t ScreenRotation = 0;
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

	bool ReverbEnabled = false;
	uint32_t ReverbStrength = 0;
	uint32_t ReverbDelay = 0;

	bool CrossFeedEnabled = false;
	uint32_t CrossFeedRatio = 0;

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

	bool AudioPlayerEnableTrackLength = true;
	uint32_t AudioPlayerTrackLength = 120;
	bool AudioPlayerAutoDetectSilence = true;
	uint32_t AudioPlayerSilenceDelay = 3;
};

enum class ControllerType
{
	None,

	//SNES controllers
	SnesController,
	SnesMouse,
	SuperScope,
	Multitap,

	//NES controllers
	NesController,
	FamicomController,
	FamicomControllerP2,
	NesZapper,
	NesArkanoidController,
	PowerPadSideA,
	PowerPadSideB,
	SuborMouse,
	VirtualBoyController,

	//NES/Famicon expansion devices
	FourScore,
	FamicomZapper,
	TwoPlayerAdapter,
	FourPlayerAdapter,
	FamicomArkanoidController,
	OekaKidsTablet,
	FamilyTrainerMatSideA,
	FamilyTrainerMatSideB,
	KonamiHyperShot,
	FamilyBasicKeyboard,
	PartyTap,
	Pachinko,
	ExcitingBoxing,
	JissenMahjong,
	SuborKeyboard,
	BarcodeBattler,
	HoriTrack,
	BandaiHyperShot,
	AsciiTurboFile,
	BattleBox,

	//NES cart input devices
	BandaiMicrophone,
	DatachBarcodeReader,

	//Game Boy
	GameboyController,
	GameboyAccelerometer,

	//PC Engine
	PceController,
	PceTurboTap,
	PceAvenuePad6,

	//SMS
	SmsController,
	SmsLightPhaser,
	ColecoVisionController,

	//GBA
	GbaController,

	//WS
	WsController,
	WsControllerVertical
};

struct KeyMapping
{
	uint16_t A = 0;
	uint16_t B = 0;
	uint16_t X = 0;
	uint16_t Y = 0;
	uint16_t L = 0;
	uint16_t R = 0;
	uint16_t Up = 0;
	uint16_t Down = 0;
	uint16_t Left = 0;
	uint16_t Right = 0;
	uint16_t Start = 0;
	uint16_t Select = 0;
	uint16_t U = 0;
	uint16_t D = 0;

	uint16_t TurboA = 0;
	uint16_t TurboB = 0;
	uint16_t TurboX = 0;
	uint16_t TurboY = 0;
	uint16_t TurboL = 0;
	uint16_t TurboR = 0;
	uint16_t TurboSelect = 0;
	uint16_t TurboStart = 0;
	
	uint16_t GenericKey1 = 0;

	uint16_t CustomKeys[100] = {};

	bool HasKeySet()
	{
		if(A || B || X || Y || L || R || U || D || Up || Down || Left || Right || Start || Select || TurboA || TurboB || TurboX || TurboY || TurboL || TurboR || TurboStart || TurboSelect || GenericKey1) {
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
	ControllerType Type = ControllerType::None;
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
	bool DisplayInputPort[8] = { };
	bool DisplayInputHorizontally = true;

	double ForceFeedbackIntensity = 1.0;
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
	Pal = 2,
	Dendy = 3,
	NtscJapan = 4
};

enum class ConsoleType
{
	Snes = 0,
	Gameboy = 1,
	Nes = 2,
	PcEngine = 3,
	Sms = 4,
	Gba = 5,
	Ws = 6
};

enum class GameboyModel
{
	AutoFavorGbc,
	AutoFavorSgb,
	AutoFavorGb,
	Gameboy,
	GameboyColor,
	SuperGameboy
};

struct EmulationConfig
{
	uint32_t EmulationSpeed = 100;
	uint32_t TurboSpeed = 300;
	uint32_t RewindSpeed = 100;

	uint32_t RunAheadFrames = 0;
};

struct OverscanDimensions
{
	uint32_t Left = 0;
	uint32_t Right = 0;
	uint32_t Top = 0;
	uint32_t Bottom = 0;
};

struct GameConfig
{
	uint32_t DipSwitches = 0;

	bool OverrideOverscan = false;
	OverscanDimensions Overscan = {};
};

struct GameboyConfig
{
	ControllerConfig Controller;

	GameboyModel Model = GameboyModel::AutoFavorGbc;
	bool UseSgb2 = true;

	bool BlendFrames = true;
	bool GbcAdjustColors = true;
	
	bool DisableBackground = false;
	bool DisableSprites = false;
	bool HideSgbBorders = false;

	RamState RamPowerOnState = RamState::Random;
	bool AllowInvalidInput = false;

	uint32_t BgColors[4] = { 0xFFFFFF, 0xB0B0B0, 0x686868, 0x000000 };
	uint32_t Obj0Colors[4] = { 0xFFFFFF, 0xB0B0B0, 0x686868, 0x000000 };
	uint32_t Obj1Colors[4] = { 0xFFFFFF, 0xB0B0B0, 0x686868, 0x000000 };

	uint32_t Square1Vol = 100;
	uint32_t Square2Vol = 100;
	uint32_t NoiseVol = 100;
	uint32_t WaveVol = 100;
};

enum class GbaSaveType
{
	AutoDetect,
	None,
	Sram,
	EepromUnknown,
	Eeprom512,
	Eeprom8192,
	Flash64,
	Flash128
};

enum class GbaRtcType
{
	AutoDetect = 0,
	Enabled = 1,
	Disabled = 2,
};

enum class GbaCartridgeType
{
	Default,
	TiltSensor
};

struct GbaConfig
{
	ControllerConfig Controller;

	bool SkipBootScreen = false;
	bool DisableFrameSkipping = false;

	bool BlendFrames = true;
	bool GbaAdjustColors = true;

	bool HideBgLayers[4] = {};
	bool DisableSprites = false;

	RamState RamPowerOnState = RamState::AllZeros;
	GbaSaveType SaveType = GbaSaveType::AutoDetect;
	GbaRtcType RtcType = GbaRtcType::AutoDetect;
	bool AllowInvalidInput = false;
	bool EnableMgbaLogApi = false;

	uint32_t ChannelAVol = 100;
	uint32_t ChannelBVol = 100;
	uint32_t Square1Vol = 100;
	uint32_t Square2Vol = 100;
	uint32_t NoiseVol = 100;
	uint32_t WaveVol = 100;
};

enum class PceConsoleType
{
	Auto,
	PcEngine,
	SuperGrafx,
	TurboGrafx
};

enum class PceCdRomType
{
	CdRom,
	SuperCdRom,
	Arcade
};

struct PcEngineConfig
{
	ControllerConfig Port1;
	ControllerConfig Port1SubPorts[5];

	bool AllowInvalidInput = false;
	bool PreventSelectRunReset = false;

	PceConsoleType ConsoleType = PceConsoleType::Auto;
	PceCdRomType CdRomType = PceCdRomType::Arcade;
	bool EnableCdRomForHuCardGames = false;
	bool DisableCdRomSaveRamForHuCardGames = false;

	RamState RamPowerOnState = RamState::Random;
	bool EnableRandomPowerOnState = false;

	uint32_t ChannelVol[6] = { 100, 100, 100, 100, 100, 100 };
	uint32_t CdAudioVolume = 100;
	uint32_t AdpcmVolume = 100;
	bool UseHuC6280aAudio = true;

	bool RemoveSpriteLimit = false;
	bool DisableSprites = false;
	bool DisableSpritesVdc2 = false;
	bool DisableBackground = false;
	bool DisableBackgroundVdc2 = false;
	bool DisableFrameSkipping = false;
	bool ForceFixedResolution = false;

	OverscanDimensions Overscan = {};

	uint32_t Palette[512] = { };
};

enum class DspInterpolationType
{
	Gauss,
	Cubic,
	Sinc,
	None
};

struct SnesConfig
{
	ControllerConfig Port1;
	ControllerConfig Port2;

	ControllerConfig Port1SubPorts[4];
	ControllerConfig Port2SubPorts[4];

	ConsoleRegion Region = ConsoleRegion::Auto;

	bool AllowInvalidInput = false;
	bool BlendHighResolutionModes = false;
	bool HideBgLayer1 = false;
	bool HideBgLayer2 = false;
	bool HideBgLayer3 = false;
	bool HideBgLayer4 = false;
	bool HideSprites = false;
	bool DisableFrameSkipping = false;
	bool ForceFixedResolution = false;

	OverscanDimensions Overscan = {};

	DspInterpolationType InterpolationType = DspInterpolationType::Gauss;
	uint32_t ChannelVolumes[8] = { 100, 100, 100, 100, 100, 100, 100, 100 };

	bool EnableRandomPowerOnState = false;
	bool EnableStrictBoardMappings = false;
	RamState RamPowerOnState = RamState::Random;
	int32_t SpcClockSpeedAdjustment = 0;

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
	MainSystemOnly = 1,
	SubSystemOnly = 2
};

enum class NesConsoleType
{
	Nes001,
	Nes101,
	Hvc001,
	Hvc101
};

struct NesConfig
{
	ControllerConfig Port1;
	ControllerConfig Port2;
	ControllerConfig ExpPort;

	ControllerConfig Port1SubPorts[4];
	ControllerConfig ExpPortSubPorts[4];

	ControllerConfig MapperInput;

	uint32_t LightDetectionRadius = 0;
	bool AutoConfigureInput = true;

	ConsoleRegion Region = ConsoleRegion::Auto;
	bool EnableHdPacks = true;
	bool DisableGameDatabase = false;
	bool FdsAutoLoadDisk = true;
	bool FdsFastForwardOnLoad = false;
	bool FdsAutoInsertDisk = false;
	VsDualOutputOption VsDualVideoOutput = VsDualOutputOption::Both;
	VsDualOutputOption VsDualAudioOutput = VsDualOutputOption::Both;

	bool SpritesEnabled = true;
	bool BackgroundEnabled = true;
	bool ForceBackgroundFirstColumn = false;
	bool ForceSpritesFirstColumn = false;
	bool RemoveSpriteLimit = false;
	bool AdaptiveSpriteLimit = false;
	bool EnablePalBorders = false;
	
	bool UseCustomVsPalette = false;
	
	OverscanDimensions NtscOverscan = {};
	OverscanDimensions PalOverscan = {};

	NesConsoleType ConsoleType = NesConsoleType::Nes001;
	bool DisablePpuReset = false;
	bool AllowInvalidInput = false;
	bool DisableGameGenieBusConflicts = false;
	bool DisableFlashSaves = false;

	bool EnableOamDecay = false;
	bool EnablePpuOamRowCorruption = false;
	bool EnablePpuSpriteEvalBug = false;
	bool DisableOamAddrBug = false;
	bool DisablePaletteRead = false;
	bool DisablePpu2004Reads = false;
	bool EnablePpu2000ScrollGlitch = false;
	bool EnablePpu2006ScrollGlitch = false;
	bool RestrictPpuAccessOnFirstFrame = false;
	bool EnableDmcSampleDuplicationGlitch = false;
	bool EnableCpuTestMode = false;

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

	int32_t InputScanline = 241;

	bool IsFullColorPalette = false;
	uint32_t UserPalette[512] = { };

	uint32_t ChannelVolumes[11] = {};
	uint32_t EpsmVolume = 100;
	uint32_t ChannelPanning[11] = {};

	StereoFilterType StereoFilter = StereoFilterType::None;
	int32_t StereoDelay = 0;
	int32_t StereoPanningAngle = 0;
	int32_t StereoCombFilterDelay = 0;
	int32_t StereoCombFilterStrength = 0;
};

enum class SmsRevision
{
	Compatibility,
	Sms1,
	Sms2
};

struct SmsConfig
{
	ControllerConfig Port1;
	ControllerConfig Port2;
	
	ConsoleRegion Region = ConsoleRegion::Auto;
	ConsoleRegion GameGearRegion = ConsoleRegion::Auto;
	RamState RamPowerOnState = RamState::Random;

	SmsRevision Revision = SmsRevision::Compatibility;

	bool AllowInvalidInput = false;
	bool UseSgPalette = false;
	bool GgBlendFrames = true;
	bool RemoveSpriteLimit = false;
	bool DisableSprites = false;
	bool DisableBackground = false;

	uint32_t ChannelVolumes[4] = {};
	uint32_t FmAudioVolume = 100;
	bool EnableFmAudio = true;

	OverscanDimensions NtscOverscan = {};
	OverscanDimensions PalOverscan = {};
	OverscanDimensions GameGearOverscan = {};
};

struct CvConfig
{
	ControllerConfig Port1;
	ControllerConfig Port2;

	ConsoleRegion Region = ConsoleRegion::Auto;
	RamState RamPowerOnState = RamState::Random;

	bool RemoveSpriteLimit = false;
	bool DisableSprites = false;
	bool DisableBackground = false;

	uint32_t ChannelVolumes[4] = {};
};

enum class WsModel : uint8_t
{
	Auto,
	Monochrome,
	Color,
	SwanCrystal
};

enum class WsAudioMode : uint8_t
{
	Headphones,
	Speakers
};

struct WsConfig
{
	ControllerConfig ControllerHorizontal;
	ControllerConfig ControllerVertical;

	WsModel Model = WsModel::Auto;
	bool UseBootRom = false;

	bool AutoRotate = false;

	bool BlendFrames = false;
	bool LcdAdjustColors = false;
	bool LcdShowIcons = false;

	bool HideBgLayers[2] = {};
	bool DisableSprites = false;

	WsAudioMode AudioMode = WsAudioMode::Headphones;
	uint32_t Channel1Vol = 100;
	uint32_t Channel2Vol = 100;
	uint32_t Channel3Vol = 100;
	uint32_t Channel4Vol = 100;
	uint32_t Channel5Vol = 100;
};

struct AudioPlayerConfig
{
	uint32_t Volume = 100;
	bool Repeat = false;
	bool Shuffle = false;
};

struct DebugConfig
{
	bool BreakOnUninitRead = false;

	bool ShowJumpLabels = false;
	bool DrawPartialFrame = false;

	bool ShowVerifiedData = false;
	bool DisassembleVerifiedData = false;

	bool ShowUnidentifiedData = false;
	bool DisassembleUnidentifiedData = false;

	bool UseLowerCaseDisassembly = false;
	bool ShowMemoryValues = false;

	bool AutoResetCdl = false;

	bool UsePredictiveBreakpoints = false;
	bool SingleBreakpointPerInstruction = false;

	bool SnesBreakOnBrk = false;
	bool SnesBreakOnCop = false;
	bool SnesBreakOnWdm = false;
	bool SnesBreakOnStp = false;
	bool SnesUseAltSpcOpNames = false;
	bool SnesIgnoreDspReadWrites = false;

	bool GbBreakOnInvalidOamAccess = false;
	bool GbBreakOnInvalidVramAccess = false;
	bool GbBreakOnDisableLcdOutsideVblank = false;
	bool GbBreakOnInvalidOpCode = false;
	bool GbBreakOnNopLoad = false;
	bool GbBreakOnOamCorruption = false;

	bool NesBreakOnBrk = false;
	bool NesBreakOnUnofficialOpCode = false;
	bool NesBreakOnCpuCrash = false;
	bool NesBreakOnBusConflict = false;
	bool NesBreakOnDecayedOamRead = false;
	bool NesBreakOnPpu2000ScrollGlitch = false;
	bool NesBreakOnPpu2006ScrollGlitch = false;
	bool NesBreakOnExtOutputMode = false;

	bool PceBreakOnBrk = false;
	bool PceBreakOnUnofficialOpCode = false;
	bool PceBreakOnInvalidVramAddress = false;

	bool SmsBreakOnNopLoad = false;

	bool GbaBreakOnNopLoad = false;
	bool GbaBreakOnInvalidOpCode = false;
	bool GbaBreakOnUnalignedMemAccess = false;
	
	bool WsBreakOnInvalidOpCode = false;

	bool ScriptAllowIoOsAccess = false;
	bool ScriptAllowNetworkAccess = false;
	uint32_t ScriptTimeout = 1;
};

enum class HudDisplaySize
{
	Fixed,
	Scaled,
};

struct PreferencesConfig
{
	bool ShowFps = false;
	bool ShowFrameCounter = false;
	bool ShowGameTimer = false;
	bool ShowLagCounter = false;
	bool ShowDebugInfo = false;
	bool DisableOsd = false;
	bool AllowBackgroundInput = false;
	bool PauseOnMovieEnd = false;
	bool ShowMovieIcons = false;
	bool ShowTurboRewindIcons = false;
	bool DisableGameSelectionScreen = false;

	HudDisplaySize HudSize = HudDisplaySize::Fixed;

	uint32_t AutoSaveStateDelay = 5;
	uint32_t RewindBufferSize = 300;

	const char* SaveFolderOverride = nullptr;
	const char* SaveStateFolderOverride = nullptr;
	const char* ScreenshotFolderOverride = nullptr;
};

struct FrameInfo
{
	uint32_t Width;
	uint32_t Height;
};

struct HudScaleFactors
{
	double X;
	double Y;
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

	ExecReset,
	ExecPowerCycle,
	ExecReloadRom,
	ExecPowerOff,

	SetScale1x,
	SetScale2x,
	SetScale3x,
	SetScale4x,
	SetScale5x,
	SetScale6x,
	SetScale7x,
	SetScale8x,
	SetScale9x,
	SetScale10x,
	ToggleFullscreen,
	ToggleFps,
	ToggleGameTimer,
	ToggleFrameCounter,
	ToggleLagCounter,
	ToggleOsd,
	ToggleAlwaysOnTop,
	ToggleDebugInfo,
	
	ToggleAudio,
	IncreaseVolume,
	DecreaseVolume,

	PreviousTrack,
	NextTrack,

	ToggleBgLayer1,
	ToggleBgLayer2,
	ToggleBgLayer3,
	ToggleBgLayer4,
	ToggleSprites1,
	ToggleSprites2,
	EnableAllLayers,

	ResetLagCounter,

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
	LoadLastSession,

	OpenFile,

	InputBarcode,
	LoadTape,
	RecordTape,
	StopRecordTape,

	//NES
	FdsSwitchDiskSide,
	FdsEjectDisk,
	FdsInsertDiskNumber,
	FdsInsertNextDisk,
	VsServiceButton,
	VsServiceButton2,
	VsInsertCoin1,
	VsInsertCoin2,
	VsInsertCoin3,
	VsInsertCoin4,
	StartRecordHdPack,
	StopRecordHdPack,

	ShortcutCount,
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

enum class DebuggerFlags
{
	SnesDebuggerEnabled = (1 << 0),
	SpcDebuggerEnabled = (1 << 1),
	Sa1DebuggerEnabled = (1 << 2),
	GsuDebuggerEnabled = (1 << 3),
	NecDspDebuggerEnabled = (1 << 4),
	Cx4DebuggerEnabled = (1 << 5),
	St018DebuggerEnabled = (1 << 6),
	GbDebuggerEnabled = (1 << 7),
	NesDebuggerEnabled = (1 << 8),
	PceDebuggerEnabled = (1 << 9),
	SmsDebuggerEnabled = (1 << 10),
	GbaDebuggerEnabled = (1 << 11),
	WsDebuggerEnabled = (1 << 12),
};
