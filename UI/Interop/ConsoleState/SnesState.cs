using System;
using System.Runtime.InteropServices;

namespace Mesen.Interop;

public enum SnesCpuStopState : byte
{
	Running = 0,
	Stopped = 1,
	WaitingForIrq = 2
}

[Flags]
public enum SnesCpuFlags : byte
{
	Carry = 0x01,
	Zero = 0x02,
	IrqDisable = 0x04,
	Decimal = 0x08,
	IndexMode8 = 0x10,
	MemoryMode8 = 0x20,
	Overflow = 0x40,
	Negative = 0x80
}

[Flags]
public enum SpcFlags : byte
{
	Carry = 0x01,
	Zero = 0x02,
	IrqEnable = 0x04,
	HalfCarry = 0x08,
	Break = 0x10,
	DirectPage = 0x20,
	Overflow = 0x40,
	Negative = 0x80
}

public enum SnesIrqSource
{
	None = 0,
	Ppu = 1,
	Coprocessor = 2
}

public struct SnesCpuState : BaseState
{
	public UInt64 CycleCount;

	public UInt16 A;
	public UInt16 X;
	public UInt16 Y;

	public UInt16 SP;
	public UInt16 D;
	public UInt16 PC;

	public byte K;
	public byte DBR;
	public SnesCpuFlags PS;
	[MarshalAs(UnmanagedType.I1)] public bool EmulationMode;

	[MarshalAs(UnmanagedType.I1)] public byte NmiFlagCounter;

	[MarshalAs(UnmanagedType.I1)] public bool IrqLock;
	[MarshalAs(UnmanagedType.I1)] public bool NeedNmi;

	public byte IrqSource;
	public byte PrevIrqSource;
	public SnesCpuStopState StopState;
}

public struct SnesPpuState : BaseState
{
	public UInt16 Cycle;
	public UInt16 Scanline;
	public UInt16 HClock;
	public UInt32 FrameCount;

	[MarshalAs(UnmanagedType.I1)] public bool ForcedBlank;
	public byte ScreenBrightness;

	public Mode7Config Mode7;

	public byte BgMode;
	[MarshalAs(UnmanagedType.I1)] public bool Mode1Bg3Priority;

	public byte MainScreenLayers;
	public byte SubScreenLayers;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	public LayerConfig[] Layers;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public WindowConfig[] Window;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
	public WindowMaskLogic[] MaskLogic;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5)]
	public byte[] WindowMaskMain;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5)]
	public byte[] WindowMaskSub;

	public UInt16 VramAddress;
	public byte VramIncrementValue;
	public byte VramAddressRemapping;
	[MarshalAs(UnmanagedType.I1)] public bool VramAddrIncrementOnSecondReg;
	public UInt16 VramReadBuffer;

	public byte Ppu1OpenBus;
	public byte Ppu2OpenBus;

	public byte CgramAddress;
	public byte InternalCgramAddress;
	public byte CgramWriteBuffer;
	[MarshalAs(UnmanagedType.I1)] public bool CgramAddressLatch;

	public byte MosaicSize;
	public byte MosaicEnabled;

	public UInt16 OamRamAddress;
	public UInt16 InternalOamRamAddress;

	public byte OamMode;
	public UInt16 OamBaseAddress;
	public UInt16 OamAddressOffset;
	[MarshalAs(UnmanagedType.I1)] public bool EnableOamPriority;

	[MarshalAs(UnmanagedType.I1)] public bool ExtBgEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool HiResMode;
	[MarshalAs(UnmanagedType.I1)] public bool ScreenInterlace;
	[MarshalAs(UnmanagedType.I1)] public bool ObjInterlace;
	[MarshalAs(UnmanagedType.I1)] public bool OverscanMode;
	[MarshalAs(UnmanagedType.I1)] public bool DirectColorMode;

	public ColorWindowMode ColorMathClipMode;
	public ColorWindowMode ColorMathPreventMode;

	[MarshalAs(UnmanagedType.I1)] public bool ColorMathAddSubscreen;
	public byte ColorMathEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool ColorMathSubtractMode;
	[MarshalAs(UnmanagedType.I1)] public bool ColorMathHalveResult;
	public UInt16 FixedColor;
}

public struct LayerConfig
{
	public UInt16 TilemapAddress;
	public UInt16 ChrAddress;

	public UInt16 HScroll;
	public UInt16 VScroll;

	[MarshalAs(UnmanagedType.I1)] public bool DoubleWidth;
	[MarshalAs(UnmanagedType.I1)] public bool DoubleHeight;

	[MarshalAs(UnmanagedType.I1)] public bool LargeTiles;
}

public struct WindowConfig
{
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
	public byte[] ActiveLayers;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
	public byte[] InvertedLayers;

	public Byte Left;
	public Byte Right;
}

public struct Mode7Config
{
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	public Int16[] Matrix;

	public Int16 HScroll;
	public Int16 VScroll;
	public Int16 CenterX;
	public Int16 CenterY;

	public Byte ValueLatch;

	[MarshalAs(UnmanagedType.I1)] public bool LargeMap;
	[MarshalAs(UnmanagedType.I1)] public bool FillWithTile0;
	[MarshalAs(UnmanagedType.I1)] public bool HorizontalMirroring;
	[MarshalAs(UnmanagedType.I1)] public bool VerticalMirroring;

	public Int16 HScrollLatch;
	public Int16 VScrollLatch;
}

public enum WindowMaskLogic
{
	Or = 0,
	And = 1,
	Xor = 2,
	Xnor = 3
}

public enum ColorWindowMode
{
	Never = 0,
	OutsideWindow = 1,
	InsideWindow = 2,
	Always = 3
}

public struct SpcTimer
{
	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	[MarshalAs(UnmanagedType.I1)] public bool TimersEnabled;
	public byte Output;
	public byte Stage0;
	public byte Stage1;
	public byte PrevStage1;
	public byte Stage2;
	public byte Target;
}

public struct SpcState : BaseState
{
	public UInt64 Cycle;
	public UInt16 PC;
	public byte A;
	public byte X;
	public byte Y;
	public byte SP;
	public SpcFlags PS;

	[MarshalAs(UnmanagedType.I1)] public bool WriteEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool RomEnabled;
	public byte InternalSpeed;
	public byte ExternalSpeed;
	[MarshalAs(UnmanagedType.I1)] public bool TimersEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool TimersDisabled;
	public SnesCpuStopState StopState;

	public byte DspReg;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	public byte[] OutputReg;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public byte[] RamReg;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	public byte[] CpuRegs;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	public byte[] NewCpuRegs;

	public SpcTimer Timer0;
	public SpcTimer Timer1;
	public SpcTimer Timer2;
}

public struct DspState
{
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
	public byte[] ExternalRegs;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 128)]
	public byte[] Regs;

	public Int32 NoiseLfsr;
	public UInt16 Counter;
	public byte Step;
	public byte OutRegBuffer;
	public byte EnvRegBuffer;
	public byte VoiceEndBuffer;

	public Int32 VoiceOutput;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public Int32[] OutSamples;

	public Int32 Pitch;
	public UInt16 SampleAddress;
	public UInt16 BrrNextAddress;
	public byte DirSampleTableAddress;
	public byte NoiseOn;
	public byte PitchModulationOn;
	public byte KeyOn;
	public byte NewKeyOn;
	public byte KeyOff;
	public byte EveryOtherSample;
	public byte SourceNumber;
	public byte BrrHeader;
	public byte BrrData;
	public byte Looped;
	public byte Adsr1;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public Int32[] EchoIn;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public Int32[] EchoOut;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8 * 2)]
	public UInt16[] EchoHistory;

	public UInt16 EchoPointer;
	public UInt16 EchoLength;
	public UInt16 EchoOffset;
	public byte EchoHistoryPos;
	public byte EchoRingBufferAddress;
	public byte EchoOn;
	[MarshalAs(UnmanagedType.I1)] public bool EchoEnabled;
}

public struct NecDspAccFlags
{
	[MarshalAs(UnmanagedType.I1)] public bool Carry;
	[MarshalAs(UnmanagedType.I1)] public bool Zero;
	[MarshalAs(UnmanagedType.I1)] public bool Overflow0;
	[MarshalAs(UnmanagedType.I1)] public bool Overflow1;
	[MarshalAs(UnmanagedType.I1)] public bool Sign0;
	[MarshalAs(UnmanagedType.I1)] public bool Sign1;
}

public struct NecDspState : BaseState
{
	public UInt64 CycleCount;
	public UInt16 A;
	public NecDspAccFlags FlagsA;
	public UInt16 B;
	public NecDspAccFlags FlagsB;
	public UInt16 TR;
	public UInt16 TRB;
	public UInt16 PC;
	public UInt16 RP;
	public UInt16 DP;
	public UInt16 DR;
	public UInt16 SR;
	public UInt16 K;
	public UInt16 L;
	public UInt16 M;
	public UInt16 N;
	public UInt16 SerialOut;
	public UInt16 SerialIn;
	public byte SP;
}


public enum ArmV3CpuMode : byte
{
	User = 0b10000,
	Fiq = 0b10001,
	Irq = 0b10010,
	Supervisor = 0b10011,
	Abort = 0b10111,
	Undefined = 0b11011,
	System = 0b11111,
}

public struct ArmV3CpuFlags
{
	public ArmV3CpuMode Mode;
	[MarshalAs(UnmanagedType.I1)] public bool FiqDisable;
	[MarshalAs(UnmanagedType.I1)] public bool IrqDisable;

	[MarshalAs(UnmanagedType.I1)] public bool Overflow;
	[MarshalAs(UnmanagedType.I1)] public bool Carry;
	[MarshalAs(UnmanagedType.I1)] public bool Zero;
	[MarshalAs(UnmanagedType.I1)] public bool Negative;
}

public struct ArmV3InstructionData
{
	public UInt32 Address;
	public UInt32 OpCode;
}

public struct ArmV3CpuPipeline
{
	public ArmV3InstructionData Fetch;
	public ArmV3InstructionData Decode;
	public ArmV3InstructionData Execute;
	[MarshalAs(UnmanagedType.I1)] public bool ReloadRequested;
	public byte Mode;
}

public struct ArmV3CpuState : BaseState
{
	public ArmV3CpuPipeline Pipeline;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
	public UInt32[] R;
	public ArmV3CpuFlags CPSR;

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

	public ArmV3CpuFlags FiqSpsr;
	public ArmV3CpuFlags IrqSpsr;
	public ArmV3CpuFlags SupervisorSpsr;
	public ArmV3CpuFlags AbortSpsr;
	public ArmV3CpuFlags UndefinedSpsr;

	public UInt64 CycleCount;
}

public struct St018State : BaseState
{
	[MarshalAs(UnmanagedType.I1)] public bool HasDataForSnes;
	public byte DataSnes;

	[MarshalAs(UnmanagedType.I1)] public bool HasDataForArm;
	public byte DataArm;

	[MarshalAs(UnmanagedType.I1)] public bool ArmReset;
	[MarshalAs(UnmanagedType.I1)] public bool Ack;
}

public struct GsuFlags
{
	[MarshalAs(UnmanagedType.I1)] public bool Zero;
	[MarshalAs(UnmanagedType.I1)] public bool Carry;
	[MarshalAs(UnmanagedType.I1)] public bool Sign;
	[MarshalAs(UnmanagedType.I1)] public bool Overflow;
	[MarshalAs(UnmanagedType.I1)] public bool Running;
	[MarshalAs(UnmanagedType.I1)] public bool RomReadPending;
	[MarshalAs(UnmanagedType.I1)] public bool Alt1;
	[MarshalAs(UnmanagedType.I1)] public bool Alt2;
	[MarshalAs(UnmanagedType.I1)] public bool ImmLow;
	[MarshalAs(UnmanagedType.I1)] public bool ImmHigh;
	[MarshalAs(UnmanagedType.I1)] public bool Prefix;
	[MarshalAs(UnmanagedType.I1)] public bool Irq;
}

public struct GsuPixelCache
{
	public byte X;
	public byte Y;
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
	public byte[] Pixels;
	public byte ValidBits;
}

public struct GsuState : BaseState
{
	public UInt64 CycleCount;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
	public UInt16[] R;

	public GsuFlags SFR;

	public byte RegisterLatch;

	public byte ProgramBank;
	public byte RomBank;
	public byte RamBank;

	[MarshalAs(UnmanagedType.I1)] public bool IrqDisabled;
	[MarshalAs(UnmanagedType.I1)] public bool HighSpeedMode;
	[MarshalAs(UnmanagedType.I1)] public bool ClockSelect;
	[MarshalAs(UnmanagedType.I1)] public bool BackupRamEnabled;
	public byte ScreenBase;

	public byte ColorGradient;
	public byte PlotBpp;
	public byte ScreenHeight;
	[MarshalAs(UnmanagedType.I1)] public bool GsuRamAccess;
	[MarshalAs(UnmanagedType.I1)] public bool GsuRomAccess;

	public UInt16 CacheBase;

	[MarshalAs(UnmanagedType.I1)] public bool PlotTransparent;
	[MarshalAs(UnmanagedType.I1)] public bool PlotDither;
	[MarshalAs(UnmanagedType.I1)] public bool ColorHighNibble;
	[MarshalAs(UnmanagedType.I1)] public bool ColorFreezeHigh;
	[MarshalAs(UnmanagedType.I1)] public bool ObjMode;

	public byte ColorReg;
	public byte SrcReg;
	public byte DestReg;

	public byte RomReadBuffer;
	public byte RomDelay;

	public byte ProgramReadBuffer;

	public UInt16 RamWriteAddress;
	public byte RamWriteValue;
	public byte RamDelay;

	public UInt16 RamAddress;

	public GsuPixelCache PrimaryCache;
	public GsuPixelCache SecondaryCache;
}

public struct Cx4Dma
{
	public UInt32 Source;
	public UInt32 Dest;
	public UInt16 Length;
	public UInt32 Pos;
	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
}

public struct Cx4Suspend
{
	public UInt32 Duration;
	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
}

public struct Cx4Cache
{
	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	public byte Page;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public byte[] Lock;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public UInt32[] Address;

	public UInt32 Base;
	public UInt16 ProgramBank;
	public byte ProgramCounter;
	public UInt16 Pos;
}

public struct Cx4Bus
{
	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	[MarshalAs(UnmanagedType.I1)] public bool Reading;
	[MarshalAs(UnmanagedType.I1)] public bool Writing;
	public byte DelayCycles;
	public UInt32 Address;
}

public struct Cx4State : BaseState
{
	public UInt64 CycleCount;
	public UInt16 PB;
	public byte PC;
	public UInt32 A;
	public UInt16 P;

	public byte SP;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
	public UInt32[] Stack;

	public UInt64 Mult;

	public UInt32 RomBuffer;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	public byte[] RamBuffer;

	public UInt32 MemoryDataReg;
	public UInt32 MemoryAddressReg;
	public UInt32 DataPointerReg;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
	public UInt32[] Regs;

	[MarshalAs(UnmanagedType.I1)] public bool Negative;
	[MarshalAs(UnmanagedType.I1)] public bool Zero;
	[MarshalAs(UnmanagedType.I1)] public bool Carry;
	[MarshalAs(UnmanagedType.I1)] public bool Overflow;
	[MarshalAs(UnmanagedType.I1)] public bool IrqFlag;

	[MarshalAs(UnmanagedType.I1)] public bool Stopped;
	[MarshalAs(UnmanagedType.I1)] public bool Locked;
	[MarshalAs(UnmanagedType.I1)] public bool IrqDisabled;

	[MarshalAs(UnmanagedType.I1)] public bool SingleRom;

	public byte RomAccessDelay;
	public byte RamAccessDelay;

	public Cx4Bus Bus;
	public Cx4Dma Dma;
	public Cx4Cache Cache;
	public Cx4Suspend Suspend;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x20)]
	public byte[] Vectors;
}

public struct Sa1State
{
	public UInt16 Sa1ResetVector;
	public UInt16 Sa1IrqVector;
	public UInt16 Sa1NmiVector;

	[MarshalAs(UnmanagedType.I1)] public bool Sa1IrqRequested;
	[MarshalAs(UnmanagedType.I1)] public bool Sa1IrqEnabled;

	[MarshalAs(UnmanagedType.I1)] public bool Sa1NmiRequested;
	[MarshalAs(UnmanagedType.I1)] public bool Sa1NmiEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool Sa1Wait;
	[MarshalAs(UnmanagedType.I1)] public bool Sa1Reset;

	[MarshalAs(UnmanagedType.I1)] public bool DmaIrqEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool TimerIrqEnabled;

	public byte Sa1MessageReceived;
	public byte CpuMessageReceived;

	public UInt16 CpuIrqVector;
	public UInt16 CpuNmiVector;
	[MarshalAs(UnmanagedType.I1)] public bool UseCpuIrqVector;
	[MarshalAs(UnmanagedType.I1)] public bool UseCpuNmiVector;

	[MarshalAs(UnmanagedType.I1)] public bool CpuIrqRequested;
	[MarshalAs(UnmanagedType.I1)] public bool CpuIrqEnabled;

	[MarshalAs(UnmanagedType.I1)] public bool CharConvIrqFlag;
	[MarshalAs(UnmanagedType.I1)] public bool CharConvIrqEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool CharConvDmaActive;
	public byte CharConvBpp;
	public byte CharConvFormat;
	public byte CharConvWidth;
	public byte CharConvCounter;

	public byte CpuBwBank;
	[MarshalAs(UnmanagedType.I1)] public bool CpuBwWriteEnabled;

	public byte Sa1BwBank;
	public byte Sa1BwMode;
	[MarshalAs(UnmanagedType.I1)] public bool Sa1BwWriteEnabled;
	public byte BwWriteProtectedArea;
	[MarshalAs(UnmanagedType.I1)] public bool BwRam2BppMode;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
	public byte[] BitmapRegister1;
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
	public byte[] BitmapRegister2;

	public byte CpuIRamWriteProtect;
	public byte Sa1IRamWriteProtect;

	public UInt32 DmaSrcAddr;
	public UInt32 DmaDestAddr;
	public UInt16 DmaSize;
	[MarshalAs(UnmanagedType.I1)] public bool DmaEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool DmaPriority;
	[MarshalAs(UnmanagedType.I1)] public bool DmaCharConv;
	[MarshalAs(UnmanagedType.I1)] public bool DmaCharConvAuto;
	public Sa1DmaDestDevice DmaDestDevice;
	public Sa1DmaSrcDevice DmaSrcDevice;
	[MarshalAs(UnmanagedType.I1)] public bool DmaRunning;
	[MarshalAs(UnmanagedType.I1)] public bool DmaIrqFlag;

	[MarshalAs(UnmanagedType.I1)] public bool HorizontalTimerEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool VerticalTimerEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool UseLinearTimer;

	public UInt16 HTimer;
	public UInt16 VTimer;
	public UInt32 LinearTimerValue;

	public Sa1MathOp MathOp;
	public UInt16 MultiplicandDividend;
	public UInt16 MultiplierDivisor;
	public UInt64 MathOpResult;
	public byte MathOverflow;

	[MarshalAs(UnmanagedType.I1)] public bool VarLenAutoInc;
	public byte VarLenBitCount;
	public UInt32 VarLenAddress;
	public byte VarLenCurrentBit;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	public byte[] Banks;
}

public enum Sa1MathOp
{
	Mul = 0,
	Div = 1,
	Sum = 2
}

public enum Sa1DmaSrcDevice
{
	PrgRom = 0,
	BwRam = 1,
	InternalRam = 2,
	Reserved = 3
}

public enum Sa1DmaDestDevice
{
	InternalRam = 0,
	BwRam = 1
}

public struct AluState
{
	public byte MultOperand1;
	public byte MultOperand2;
	public UInt16 MultOrRemainderResult;

	public UInt16 Dividend;
	public byte Divisor;
	public UInt16 DivResult;
}

public struct InternalRegisterState
{
	[MarshalAs(UnmanagedType.I1)] public bool EnableAutoJoypadRead;
	[MarshalAs(UnmanagedType.I1)] public bool EnableFastRom;

	[MarshalAs(UnmanagedType.I1)] public bool EnableNmi;
	[MarshalAs(UnmanagedType.I1)] public bool EnableHorizontalIrq;
	[MarshalAs(UnmanagedType.I1)] public bool EnableVerticalIrq;
	public UInt16 HorizontalTimer;
	public UInt16 VerticalTimer;

	public byte IoPortOutput;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	public UInt16[] ControllerData;
}

public struct SnesDmaControllerState
{
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
	public DmaChannelConfig[] Channels;

	public byte HdmaChannels;
}

public struct SnesState : BaseState
{
	public UInt64 MasterClock;
	public SnesCpuState Cpu;
	public SnesPpuState Ppu;
	public SpcState Spc;
	public DspState Dsp;
	public NecDspState NecDsp;
	public Sa1State Sa1;
	public GsuState Gsu;
	public Cx4State Cx4;
	public St018State St018;

	public SnesDmaControllerState Dma;
	public InternalRegisterState InternalRegs;
	public AluState Alu;

	public UInt32 WramPosition;
}

public struct SnesPpuToolsState : BaseState
{
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 239)]
	public byte[] ScanlineBgMode;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 239)]
	public Int32[] Mode7StartX;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 239)]
	public Int32[] Mode7StartY;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 239)]
	public Int32[] Mode7EndX;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 239)]
	public Int32[] Mode7EndY;
}