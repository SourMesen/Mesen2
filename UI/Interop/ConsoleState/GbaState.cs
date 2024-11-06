using System;
using System.Runtime.InteropServices;

namespace Mesen.Interop;

public struct GbaTimerState
{
	public UInt16 ReloadValue;
	public UInt16 NewReloadValue;
	public UInt16 PrescaleMask;
	public UInt16 Timer;
	public byte Control;

	public byte EnableDelay;
	[MarshalAs(UnmanagedType.I1)] public bool WritePending;
	[MarshalAs(UnmanagedType.I1)] public bool Mode;
	[MarshalAs(UnmanagedType.I1)] public bool IrqEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	[MarshalAs(UnmanagedType.I1)] public bool ProcessTimer;
}

public struct GbaTimersState
{
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	public GbaTimerState[] Timer;
}

public enum GbaDmaTrigger : byte
{
	Immediate = 0,
	VBlank = 1,
	HBlank = 2,
	Audio = 3
}

public enum GbaDmaAddrMode : byte
{
	Increment,
	Decrement,
	Fixed,
	IncrementReload
}

public struct GbaDmaChannel
{
	public UInt32 ReadValue;

	public UInt32 Destination;
	public UInt32 Source;
	public UInt16 Length;

	public UInt32 DestLatch;
	public UInt32 SrcLatch;
	public UInt16 LenLatch;

	public UInt16 Control;

	public GbaDmaAddrMode DestMode;
	public GbaDmaAddrMode SrcMode;

	[MarshalAs(UnmanagedType.I1)] public bool Repeat;
	[MarshalAs(UnmanagedType.I1)] public bool WordTransfer;
	[MarshalAs(UnmanagedType.I1)] public bool DrqMode;

	public GbaDmaTrigger Trigger;
	[MarshalAs(UnmanagedType.I1)] public bool IrqEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	[MarshalAs(UnmanagedType.I1)] public bool Active;
}

public struct GbaDmaControllerState
{
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	public GbaDmaChannel[] Ch;
}

public struct GbaApuState
{
	public sbyte DmaSampleA;
	public sbyte DmaSampleB;

	public byte VolumeControl;
	public byte GbVolume;
	public byte VolumeA;
	public byte VolumeB;

	public byte DmaSoundControl;
	[MarshalAs(UnmanagedType.I1)] public bool EnableRightA;
	[MarshalAs(UnmanagedType.I1)] public bool EnableLeftA;
	public byte TimerA;
	[MarshalAs(UnmanagedType.I1)] public bool EnableRightB;
	[MarshalAs(UnmanagedType.I1)] public bool EnableLeftB;
	public byte TimerB;

	public byte EnabledGb;
	public byte EnableLeftSq1;
	public byte EnableLeftSq2;
	public byte EnableLeftWave;
	public byte EnableLeftNoise;

	public byte EnableRightSq1;
	public byte EnableRightSq2;
	public byte EnableRightWave;
	public byte EnableRightNoise;

	public byte LeftVolume;
	public byte RightVolume;

	public byte FrameSequenceStep;

	[MarshalAs(UnmanagedType.I1)] public bool ApuEnabled;

	public UInt16 Bias;
	public byte SamplingRate;
}

public struct GbaSquareState
{
	public UInt16 Frequency;
	public UInt16 Timer;

	public UInt16 SweepTimer;
	public UInt16 SweepFreq;
	public UInt16 SweepPeriod;
	public byte SweepUpdateDelay;
	[MarshalAs(UnmanagedType.I1)] public bool SweepNegate;
	public byte SweepShift;

	[MarshalAs(UnmanagedType.I1)] public bool SweepEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool SweepNegateCalcDone;

	public byte Volume;
	public byte EnvVolume;
	[MarshalAs(UnmanagedType.I1)] public bool EnvRaiseVolume;
	public byte EnvPeriod;
	public byte EnvTimer;
	[MarshalAs(UnmanagedType.I1)] public bool EnvStopped;

	public byte Duty;

	public byte Length;
	[MarshalAs(UnmanagedType.I1)] public bool LengthEnabled;

	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	public byte DutyPos;
	public byte Output;
}

public struct GbaNoiseState
{
	public byte Volume;
	public byte EnvVolume;
	[MarshalAs(UnmanagedType.I1)] public bool EnvRaiseVolume;
	public byte EnvPeriod;
	public byte EnvTimer;
	[MarshalAs(UnmanagedType.I1)] public bool EnvStopped;

	public byte Length;
	[MarshalAs(UnmanagedType.I1)] public bool LengthEnabled;

	public UInt16 ShiftRegister;

	public byte PeriodShift;
	public byte Divisor;
	[MarshalAs(UnmanagedType.I1)] public bool ShortWidthMode;

	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	public UInt32 Timer;
	public byte Output;
}

public struct GbaWaveState
{
	[MarshalAs(UnmanagedType.I1)] public bool DacEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool DoubleLength;
	public byte SelectedBank;

	public byte SampleBuffer;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x20)]
	public byte[] Ram;

	public byte Position;

	public byte Volume;
	[MarshalAs(UnmanagedType.I1)] public bool OverrideVolume;
	public UInt16 Frequency;

	public UInt16 Length;
	[MarshalAs(UnmanagedType.I1)] public bool LengthEnabled;

	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	public UInt16 Timer;
	public byte Output;
}

public struct GbaApuDebugState
{
	public GbaApuState Common;
	public GbaSquareState Square1;
	public GbaSquareState Square2;
	public GbaWaveState Wave;
	public GbaNoiseState Noise;
}

public struct GbaControlManagerState
{
	public UInt16 KeyControl;
	public UInt16 ActiveKeys;
}

public struct GbaState : BaseState
{
	public GbaCpuState Cpu;
	public GbaPpuState Ppu;
	public GbaApuDebugState Apu;
	public GbaMemoryManagerState MemoryManager;
	public GbaDmaControllerState Dma;
	public GbaTimersState Timer;
	public GbaRomPrefetchState Prefetch;
	public GbaControlManagerState ControlManager;
}

public enum GbaCpuMode : byte
{
	User = 0b10000,
	Fiq = 0b10001,
	Irq = 0b10010,
	Supervisor = 0b10011,
	Abort = 0b10111,
	Undefined = 0b11011,
	System = 0b11111,
}

public struct GbaCpuFlags
{
	public GbaCpuMode Mode;
	[MarshalAs(UnmanagedType.I1)] public bool Thumb;
	[MarshalAs(UnmanagedType.I1)] public bool FiqDisable;
	[MarshalAs(UnmanagedType.I1)] public bool IrqDisable;

	[MarshalAs(UnmanagedType.I1)] public bool Overflow;
	[MarshalAs(UnmanagedType.I1)] public bool Carry;
	[MarshalAs(UnmanagedType.I1)] public bool Zero;
	[MarshalAs(UnmanagedType.I1)] public bool Negative;
}

public struct GbaInstructionData
{
	public UInt32 Address;
	public UInt32 OpCode;
}

public struct GbaCpuPipeline
{
	public GbaInstructionData Fetch;
	public GbaInstructionData Decode;
	public GbaInstructionData Execute;
	[MarshalAs(UnmanagedType.I1)] public bool ReloadRequested;
	public byte Mode;
}

public struct GbaCpuState : BaseState
{
	public GbaCpuPipeline Pipeline;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
	public UInt32[] R;
	public GbaCpuFlags CPSR;
	[MarshalAs(UnmanagedType.I1)] public bool Stopped;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 7)]
	public UInt32[] UserRegs;
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 7)]
	public UInt32[] FiqRegs;
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public UInt32[] IrqRegs;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public UInt32[] SupervisorRegs;
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public UInt32[] AbortRegs;
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public UInt32[] UndefinedRegs;

	public GbaCpuFlags FiqSpsr;
	public GbaCpuFlags IrqSpsr;
	public GbaCpuFlags SupervisorSpsr;
	public GbaCpuFlags AbortSpsr;
	public GbaCpuFlags UndefinedSpsr;

	public UInt64 CycleCount;
}

public struct GbaBgConfig
{
	public UInt16 Control;
	public UInt16 TilemapAddr;
	public UInt16 TilesetAddr;
	public UInt16 ScrollX;
	public UInt16 ScrollXLatch;
	public UInt16 ScrollY;
	public byte ScreenSize;
	[MarshalAs(UnmanagedType.I1)] public bool DoubleWidth;
	[MarshalAs(UnmanagedType.I1)] public bool DoubleHeight;
	public byte Priority;
	[MarshalAs(UnmanagedType.I1)] public bool Mosaic;
	[MarshalAs(UnmanagedType.I1)] public bool WrapAround;
	[MarshalAs(UnmanagedType.I1)] public bool Bpp8Mode;
	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	public byte EnableTimer;
}

public struct GbaTransformConfig
{
	public UInt32 OriginX;
	public UInt32 OriginY;

	public Int32 LatchOriginX;
	public Int32 LatchOriginY;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	public Int16[] Matrix;

	[MarshalAs(UnmanagedType.I1)] public bool PendingUpdateX;
	[MarshalAs(UnmanagedType.I1)] public bool PendingUpdateY;
}

public enum GbaPpuBlendEffect : byte
{
	None,
	AlphaBlend,
	IncreaseBrightness,
	DecreaseBrightness
}

public struct GbaWindowConfig
{
	public byte LeftX;
	public byte RightX;
	public byte TopY;
	public byte BottomY;
}

public struct GbaPpuState : BaseState
{
	public UInt32 FrameCount;
	public UInt16 Cycle;
	public UInt16 Scanline;

	public byte Control;
	public byte BgMode;
	[MarshalAs(UnmanagedType.I1)] public bool DisplayFrameSelect;
	[MarshalAs(UnmanagedType.I1)] public bool AllowHblankOamAccess;
	[MarshalAs(UnmanagedType.I1)] public bool ObjVramMappingOneDimension;
	[MarshalAs(UnmanagedType.I1)] public bool ForcedBlank;
	[MarshalAs(UnmanagedType.I1)] public bool GreenSwapEnabled;

	public byte Control2;
	public byte ObjEnableTimer;
	[MarshalAs(UnmanagedType.I1)] public bool ObjLayerEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool Window0Enabled;
	[MarshalAs(UnmanagedType.I1)] public bool Window1Enabled;
	[MarshalAs(UnmanagedType.I1)] public bool ObjWindowEnabled;

	public byte DispStat;
	[MarshalAs(UnmanagedType.I1)] public bool VblankIrqEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool HblankIrqEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool ScanlineIrqEnabled;
	public byte Lyc;

	public byte BlendMainControl;
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)] public byte[] BlendMain;
	public byte BlendSubControl;
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)] public byte[] BlendSub;
	public GbaPpuBlendEffect BlendEffect;
	public byte BlendMainCoefficient;
	public byte BlendSubCoefficient;
	public byte Brightness;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	public GbaBgConfig[] BgLayers;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public GbaTransformConfig[] Transform;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public GbaWindowConfig[] Window;

	public byte BgMosaicSizeX;
	public byte BgMosaicSizeY;
	public byte ObjMosaicSizeX;
	public byte ObjMosaicSizeY;

	public byte Window0Control;
	public byte Window1Control;
	public byte ObjWindowControl;
	public byte OutWindowControl;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5 * 6)]
	public byte[] WindowActiveLayers;
}

public struct GbaMemoryManagerState
{
	public UInt16 IE;
	public UInt16 IF;
	public UInt16 NewIE;
	public UInt16 NewIF;

	public UInt16 WaitControl;
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public byte[] PrgWaitStates0;
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public byte[] PrgWaitStates1;
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public byte[] PrgWaitStates2;
	public byte SramWaitStates;
	[MarshalAs(UnmanagedType.I1)] public bool PrefetchEnabled;
	public byte IME;
	public byte NewIME;
	public byte IrqUpdateCounter;
	public byte IrqPending;
	public byte IrqLine;
	[MarshalAs(UnmanagedType.I1)] public bool BusLocked;
	[MarshalAs(UnmanagedType.I1)] public bool StopMode;
	[MarshalAs(UnmanagedType.I1)] public bool PostBootFlag;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	public byte[] BootRomOpenBus;
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	public byte[] InternalOpenBus;
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	public byte[] IwramOpenBus;
}

public struct GbaRomPrefetchState
{
	public UInt32 ReadAddr;
	public UInt32 PrefetchAddr;
	public byte ClockCounter;
	public byte BoundaryCyclePenalty;
	[MarshalAs(UnmanagedType.I1)] public bool Suspended;
	[MarshalAs(UnmanagedType.I1)] public bool WasFilled;
	[MarshalAs(UnmanagedType.I1)] public bool Started;
	[MarshalAs(UnmanagedType.I1)] public bool Sequential;
}
