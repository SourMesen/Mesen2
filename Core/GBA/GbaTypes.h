#pragma once
#include "pch.h"
#include "Shared/MemoryType.h"
#include "Shared/BaseState.h"
#include "Shared/ArmEnums.h"
#include "Utilities/Serializer.h"

enum class GbaCpuMode : uint8_t
{
	User = 0b10000,
	Fiq = 0b10001,
	Irq = 0b10010,
	Supervisor = 0b10011,
	Abort = 0b10111,
	Undefined = 0b11011,
	System = 0b11111,
};

enum class GbaCpuVector : uint32_t
{
	Undefined = 0x04,
	SoftwareIrq = 0x08,
	AbortPrefetch = 0x0C,
	AbortData = 0x10,
	Irq = 0x18,
	Fiq = 0x1C,
};

typedef uint8_t GbaAccessModeVal;

namespace GbaAccessMode
{
	enum Mode
	{
		Sequential = (1 << 0),
		Word = (1 << 1),
		HalfWord = (1 << 2),
		Byte = (1 << 3),
		Signed = (1 << 4),
		NoRotate = (1 << 5),
		Prefetch = (1 << 6),
		Dma = (1 << 7)
	};
}

struct GbaCpuFlags
{
	GbaCpuMode Mode;
	bool Thumb;
	bool FiqDisable;
	bool IrqDisable;

	bool Overflow;
	bool Carry;
	bool Zero;
	bool Negative;
	
	uint32_t ToInt32()
	{
		return (
			(Negative << 31) |
			(Zero << 30) |
			(Carry << 29) |
			(Overflow << 28) |
			
			(IrqDisable << 7) |
			(FiqDisable << 6) |
			(Thumb << 5) |
			(uint8_t)Mode
		);
	}
};

struct GbaInstructionData
{
	uint32_t Address;
	uint32_t OpCode;
};

struct GbaCpuPipeline
{
	GbaInstructionData Fetch;
	GbaInstructionData Decode;
	GbaInstructionData Execute;
	bool ReloadRequested;
	GbaAccessModeVal Mode;
};

struct GbaCpuState : BaseState
{
	GbaCpuPipeline Pipeline;
	uint32_t R[16];
	GbaCpuFlags CPSR;
	bool Stopped;

	uint32_t UserRegs[7];
	uint32_t FiqRegs[7];
	uint32_t IrqRegs[2];

	uint32_t SupervisorRegs[2];
	uint32_t AbortRegs[2];
	uint32_t UndefinedRegs[2];

	GbaCpuFlags FiqSpsr;
	GbaCpuFlags IrqSpsr;
	GbaCpuFlags SupervisorSpsr;
	GbaCpuFlags AbortSpsr;
	GbaCpuFlags UndefinedSpsr;
	
	uint64_t CycleCount;
};

struct GbaBgConfig
{
	uint16_t Control;
	uint16_t TilemapAddr;
	uint16_t TilesetAddr;
	uint16_t ScrollX;
	uint16_t ScrollXLatch;
	uint16_t ScrollY;
	uint8_t ScreenSize;
	bool DoubleWidth;
	bool DoubleHeight;
	uint8_t Priority;
	bool Mosaic;
	bool WrapAround;
	bool Bpp8Mode;
	bool Enabled;
	uint8_t EnableTimer;
};

struct GbaTransformConfig
{
	uint32_t OriginX;
	uint32_t OriginY;

	int32_t LatchOriginX;
	int32_t LatchOriginY;

	int16_t Matrix[4];

	bool PendingUpdateX;
	bool PendingUpdateY;
};

struct GbaWindowConfig
{
	uint8_t LeftX;
	uint8_t RightX;
	uint8_t TopY;
	uint8_t BottomY;
};

enum class GbaPpuBlendEffect : uint8_t
{
	None,
	AlphaBlend,
	IncreaseBrightness,
	DecreaseBrightness
};

enum class GbaPpuObjMode : uint8_t
{
	Normal,
	Blending,
	Window,
	Invalid
};

namespace GbaPpuMemAccess
{
	enum GbaPpuMemAccess : uint8_t
	{
		None = 0,
		Palette = 1,
		Vram = 2,
		Oam = 4,
		VramObj = 8
	};
}

struct GbaPpuState : BaseState
{
	uint32_t FrameCount;
	uint16_t Cycle;
	uint16_t Scanline;

	uint8_t Control;
	uint8_t BgMode;
	bool DisplayFrameSelect;
	bool AllowHblankOamAccess;
	bool ObjVramMappingOneDimension;
	bool ForcedBlank;
	bool GreenSwapEnabled;

	uint8_t Control2;
	uint8_t ObjEnableTimer;
	bool ObjLayerEnabled;
	bool Window0Enabled;
	bool Window1Enabled;
	bool ObjWindowEnabled;

	uint8_t DispStat;
	bool VblankIrqEnabled;
	bool HblankIrqEnabled;
	bool ScanlineIrqEnabled;
	uint8_t Lyc;

	uint8_t BlendMainControl;
	bool BlendMain[6];
	uint8_t BlendSubControl;
	bool BlendSub[6];
	GbaPpuBlendEffect BlendEffect;
	uint8_t BlendMainCoefficient;
	uint8_t BlendSubCoefficient;
	uint8_t Brightness;

	GbaBgConfig BgLayers[4];
	GbaTransformConfig Transform[2];
	GbaWindowConfig Window[2];

	uint8_t BgMosaicSizeX;
	uint8_t BgMosaicSizeY;
	uint8_t ObjMosaicSizeX;
	uint8_t ObjMosaicSizeY;

	uint8_t Window0Control;
	uint8_t Window1Control;
	uint8_t ObjWindowControl;
	uint8_t OutWindowControl;
	bool WindowActiveLayers[5][6];
};

struct GbaMemoryManagerState
{
	uint16_t IE; //200-201
	uint16_t IF; //202-203
	uint16_t NewIE;
	uint16_t NewIF;

	uint16_t WaitControl; //204-205
	uint8_t PrgWaitStates0[2] = { 5,3 };
	uint8_t PrgWaitStates1[2] = { 5,5 };
	uint8_t PrgWaitStates2[2] = { 5,9 };
	uint8_t SramWaitStates = 5;
	bool PrefetchEnabled;
	uint8_t IME; //208-20B
	uint8_t NewIME;
	uint8_t IrqUpdateCounter;
	uint8_t IrqLine;
	uint8_t IrqPending;
	bool BusLocked;
	bool StopMode;
	bool PostBootFlag;

	uint8_t BootRomOpenBus[4];
	uint8_t InternalOpenBus[4];
	uint8_t IwramOpenBus[4];
};

struct GbaRomPrefetchState
{
	uint32_t ReadAddr;
	uint32_t PrefetchAddr;
	uint8_t ClockCounter;
	uint8_t BoundaryCyclePenalty;
	bool Suspended;
	bool WasFilled;
	bool Started;
	bool Sequential;
};

struct GbaTimerState
{
	uint16_t ReloadValue;
	uint16_t NewReloadValue;
	uint16_t PrescaleMask;
	uint16_t Timer;
	uint8_t Control;

	uint8_t EnableDelay;
	bool WritePending;
	bool Mode;
	bool IrqEnabled;
	bool Enabled;
	bool ProcessTimer;
};

struct GbaTimersState
{
	GbaTimerState Timer[4];
};

enum class GbaDmaTrigger : uint8_t
{
	Immediate = 0,
	VBlank = 1,
	HBlank = 2,
	Special = 3
};

enum class GbaDmaAddrMode : uint8_t
{
	Increment,
	Decrement,
	Fixed,
	IncrementReload
};

struct GbaDmaChannel
{
	uint32_t ReadValue;

	uint32_t Destination;
	uint32_t Source;
	uint16_t Length;

	uint32_t DestLatch;
	uint32_t SrcLatch;
	uint16_t LenLatch;

	uint16_t Control;

	GbaDmaAddrMode DestMode;
	GbaDmaAddrMode SrcMode;

	bool Repeat;
	bool WordTransfer;
	bool DrqMode;

	GbaDmaTrigger Trigger;
	bool IrqEnabled;
	bool Enabled;
	bool Active;

	bool Pending;
};

struct GbaDmaControllerState
{
	GbaDmaChannel Ch[4];
};

struct GbaSquareState
{
	uint16_t Frequency;
	uint16_t Timer;

	uint16_t SweepTimer;
	uint16_t SweepFreq;
	uint16_t SweepPeriod;
	uint8_t SweepUpdateDelay;
	bool SweepNegate;
	uint8_t SweepShift;

	bool SweepEnabled;
	bool SweepNegateCalcDone;

	uint8_t Volume;
	uint8_t EnvVolume;
	bool EnvRaiseVolume;
	uint8_t EnvPeriod;
	uint8_t EnvTimer;
	bool EnvStopped;

	uint8_t Duty;

	uint8_t Length;
	bool LengthEnabled;

	bool Enabled;
	uint8_t DutyPos;
	uint8_t Output;
};

struct GbaNoiseState
{
	uint8_t Volume;
	uint8_t EnvVolume;
	bool EnvRaiseVolume;
	uint8_t EnvPeriod;
	uint8_t EnvTimer;
	bool EnvStopped;

	uint8_t Length;
	bool LengthEnabled;

	uint16_t ShiftRegister;

	uint8_t PeriodShift;
	uint8_t Divisor;
	bool ShortWidthMode;

	bool Enabled;
	uint32_t Timer;
	uint8_t Output;
};

struct GbaWaveState
{
	bool DacEnabled;
	bool DoubleLength;
	uint8_t SelectedBank;

	uint8_t SampleBuffer;
	uint8_t Ram[0x20];
	uint8_t Position;

	uint8_t Volume;
	bool OverrideVolume;
	uint16_t Frequency;

	uint16_t Length;
	bool LengthEnabled;

	bool Enabled;
	uint16_t Timer;
	uint8_t Output;
};

struct GbaApuState
{
	int8_t DmaSampleA;
	int8_t DmaSampleB;

	uint8_t VolumeControl;
	uint8_t GbVolume;
	uint8_t VolumeA;
	uint8_t VolumeB;
	
	uint8_t DmaSoundControl;
	bool EnableRightA;
	bool EnableLeftA;
	uint8_t TimerA;
	bool EnableRightB;
	bool EnableLeftB;
	uint8_t TimerB;

	uint8_t EnabledGb;
	uint8_t EnableLeftSq1;
	uint8_t EnableLeftSq2;
	uint8_t EnableLeftWave;
	uint8_t EnableLeftNoise;

	uint8_t EnableRightSq1;
	uint8_t EnableRightSq2;
	uint8_t EnableRightWave;
	uint8_t EnableRightNoise;

	uint8_t LeftVolume;
	uint8_t RightVolume;

	uint8_t FrameSequenceStep;

	bool ApuEnabled;

	uint16_t Bias;
	uint8_t SamplingRate;
};

struct GbaSerialState
{
	uint64_t StartMasterClock;
	uint64_t EndMasterClock;
	uint64_t IrqMasterClock;

	uint16_t Data[4]; //120-127
	
	uint16_t Control; //128-129
	bool InternalShiftClock;
	bool InternalShiftClockSpeed2MHz;
	bool Active;
	bool TransferWord;
	bool IrqEnabled;

	uint16_t SendData; //12A-12B
	uint16_t Mode; //134-135
	uint16_t JoyControl; //140-141
	uint32_t JoyReceive; //150-153
	uint32_t JoySend; //154-157
	uint8_t JoyStatus; //158
};

struct GbaControlManagerState
{
	uint16_t KeyControl;
	uint16_t ActiveKeys;
};

struct GbaApuDebugState
{
	GbaApuState Common;
	GbaSquareState Square1;
	GbaSquareState Square2;
	GbaWaveState Wave;
	GbaNoiseState Noise;
};

struct GbaState
{
	GbaCpuState Cpu;
	GbaPpuState Ppu;
	GbaApuDebugState Apu;
	GbaMemoryManagerState MemoryManager;
	GbaDmaControllerState Dma;
	GbaTimersState Timer;
	GbaRomPrefetchState Prefetch;
	GbaControlManagerState ControlManager;
};

enum class GbaThumbOpCategory
{
	MoveShiftedRegister,
	AddSubtract,
	MoveCmpAddSub,
	AluOperation,
	HiRegBranchExch,
	PcRelLoad,
	LoadStoreRegOffset,
	LoadStoreSignExtended,
	LoadStoreImmOffset,
	LoadStoreHalfWord,
	SpRelLoadStore,
	LoadAddress,
	AddOffsetToSp,
	PushPopReg,
	MultipleLoadStore,
	ConditionalBranch,
	SoftwareInterrupt,
	UnconditionalBranch,
	LongBranchLink,

	InvalidOp,
};

enum class GbaIrqSource
{
	None = 0,
	LcdVblank = 1 << 0,
	LcdHblank = 1 << 1,
	LcdScanlineMatch = 1 << 2,

	Timer0 = 1 << 3,
	Timer1 = 1 << 4,
	Timer2 = 1 << 5,
	Timer3 = 1 << 6,

	Serial = 1 << 7,

	DmaChannel0 = 1 << 8,
	DmaChannel1 = 1 << 9,
	DmaChannel2 = 1 << 10,
	DmaChannel3 = 1 << 11,

	Keypad = 1 << 12,
	External = 1 << 13
};

class GbaConstants
{
public:
	static constexpr uint32_t ScreenWidth = 240;
	static constexpr uint32_t ScreenHeight = 160;
	static constexpr uint32_t PixelCount = GbaConstants::ScreenWidth * GbaConstants::ScreenHeight;
};