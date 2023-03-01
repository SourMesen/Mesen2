using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace Mesen.Interop
{
	public interface BaseState
	{
	}

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

		[MarshalAs(UnmanagedType.I1)] public bool NmiFlag;
		[MarshalAs(UnmanagedType.I1)] public bool PrevNmiFlag;

		[MarshalAs(UnmanagedType.I1)] public bool IrqLock;
		[MarshalAs(UnmanagedType.I1)] public bool PrevNeedNmi;
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

		public SpcTimer Timer0;
		public SpcTimer Timer1;
		public SpcTimer Timer2;
	}

	public struct DspState
	{
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

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8*2)]
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

	public struct DebugSa1State
	{
		public SnesCpuState Cpu;
		public Sa1State Sa1;
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

	public enum GbMemoryType
	{
		None = 0,
		PrgRom = (int)MemoryType.GbPrgRom,
		WorkRam = (int)MemoryType.GbWorkRam,
		CartRam = (int)MemoryType.GbCartRam,
		BootRom = (int)MemoryType.GbBootRom,
	}

	public enum GbRegisterAccess
	{
		None = 0,
		Read = 1,
		Write = 2,
		ReadWrite = 3
	}

	public struct GbMemoryManagerState
	{
		public UInt64 ApuCycleCount;

		public byte CgbWorkRamBank;
		[MarshalAs(UnmanagedType.I1)] public bool CgbSwitchSpeedRequest;
		[MarshalAs(UnmanagedType.I1)] public bool CgbHighSpeed;

		public byte CgbRegFF72;
		public byte CgbRegFF73;
		public byte CgbRegFF74;
		public byte CgbRegFF75;

		[MarshalAs(UnmanagedType.I1)] public bool DisableBootRom;
		public byte IrqRequests;
		public byte IrqEnabled;

		public byte SerialData;
		public byte SerialControl;
		public byte SerialBitCount;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x100)]
		public byte[] IsReadRegister;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x100)]
		public byte[] IsWriteRegister;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x100)]
		public GbMemoryType[] MemoryType;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x100)]
		public UInt32[] MemoryOffset;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x100)]
		public GbRegisterAccess[] MemoryAccessType;
	}

	public struct GbControlManagerState
	{
		public byte InputSelect;
	}

	public struct GbDmaControllerState
	{
		public byte OamDmaSource;
		public byte DmaStartDelay;
		public byte InternalDest;
		public byte DmaCounter;
		public byte DmaReadBuffer;
		[MarshalAs(UnmanagedType.I1)] public bool OamDmaRunning;

		public UInt16 CgbDmaSource;
		public UInt16 CgbDmaDest;
		public byte CgbDmaLength;
		[MarshalAs(UnmanagedType.I1)] public bool CgbHdmaDone;
		[MarshalAs(UnmanagedType.I1)] public bool CgbHdmaRunning;
	};

	public struct GbTimerState
	{
		public UInt16 Divider;

		[MarshalAs(UnmanagedType.I1)] public bool NeedReload;
		[MarshalAs(UnmanagedType.I1)] public bool Reloaded;
		public byte Counter;
		public byte Modulo;

		public byte Control;
		[MarshalAs(UnmanagedType.I1)] public bool TimerEnabled;
		public UInt16 TimerDivider;
	};

	public enum GbType
	{
		Gb = 0,
		Cgb = 1,
	}

	public struct GbState : BaseState
	{
		public GbType Type;
		public GbCpuState Cpu;
		public GbPpuState Ppu;
		public GbApuDebugState Apu;
		public GbMemoryManagerState MemoryManager;
		public GbTimerState Timer;
		public GbDmaControllerState Dma;
		public GbControlManagerState ControlManager;
		[MarshalAs(UnmanagedType.I1)] public bool HasBattery;
	}

	[Flags]
	public enum GameboyFlags : byte
	{
		Carry = 0x10,
		HalfCarry = 0x20,
		AddSub = 0x40,
		Zero = 0x80
	}

	public struct GbCpuState : BaseState
	{
		public UInt64 CycleCount;
		public UInt16 PC;
		public UInt16 SP;

		public byte A;
		public byte Flags;

		public byte B;
		public byte C;
		public byte D;
		public byte E;

		public byte H;
		public byte L;

		[MarshalAs(UnmanagedType.I1)] public bool EiPending;
		[MarshalAs(UnmanagedType.I1)] public bool IME;
		[MarshalAs(UnmanagedType.I1)] public bool Halted;
	}

	public enum PpuMode
	{
		HBlank,
		VBlank,
		OamEvaluation,
		Drawing,
		NoIrq
	}

	public struct GbPpuState : BaseState
	{
		public byte Scanline;
		public UInt16 Cycle;
		public UInt16 IdleCycles;
		public PpuMode Mode;
		public PpuMode IrqMode;
		[MarshalAs(UnmanagedType.I1)] public bool StatIrqFlag;

		public byte Ly;
		public Int16 LyForCompare;

		public byte LyCompare;
		[MarshalAs(UnmanagedType.I1)] public bool LyCoincidenceFlag;
		public byte BgPalette;
		public byte ObjPalette0;
		public byte ObjPalette1;
		public byte ScrollX;
		public byte ScrollY;
		public byte WindowX;
		public byte WindowY;

		public byte Control;
		[MarshalAs(UnmanagedType.I1)] public bool LcdEnabled;
		[MarshalAs(UnmanagedType.I1)] public bool WindowTilemapSelect;
		[MarshalAs(UnmanagedType.I1)] public bool WindowEnabled;
		[MarshalAs(UnmanagedType.I1)] public bool BgTileSelect;
		[MarshalAs(UnmanagedType.I1)] public bool BgTilemapSelect;
		[MarshalAs(UnmanagedType.I1)] public bool LargeSprites;
		[MarshalAs(UnmanagedType.I1)] public bool SpritesEnabled;
		[MarshalAs(UnmanagedType.I1)] public bool BgEnabled;

		public byte Status;
		public UInt32 FrameCount;

		[MarshalAs(UnmanagedType.I1)] public bool CgbEnabled;
		public byte CgbVramBank;

		public byte CgbBgPalPosition;
		[MarshalAs(UnmanagedType.I1)] public bool CgbBgPalAutoInc;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4*8)]
		public UInt16[] CgbBgPalettes;

		public byte CgbObjPalPosition;
		[MarshalAs(UnmanagedType.I1)] public bool CgbObjPalAutoInc;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4 * 8)]
		public UInt16[] CgbObjPalettes;
	}

	public struct GbSquareState
	{
		public UInt16 Frequency;
		public UInt16 Timer;
		
		public UInt16 SweepTimer;
		public UInt16 SweepFreq;
		public UInt16 SweepPeriod;
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

	public struct GbNoiseState
	{
		public byte Volume;
		public byte EnvVolume;
		[MarshalAs(UnmanagedType.I1)] public bool EnvRaiseVolume;
		public byte EnvPeriod;
		public byte EnvTimer;

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

	public struct GbWaveState
	{
		[MarshalAs(UnmanagedType.I1)] public bool DacEnabled;

		public byte SampleBuffer;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
		public byte[] Ram;

		public byte Position;

		public byte Volume;
		public UInt16 Frequency;

		public UInt16 Length;
		[MarshalAs(UnmanagedType.I1)] public bool LengthEnabled;

		[MarshalAs(UnmanagedType.I1)] public bool Enabled;
		public UInt16 Timer;
		public byte Output;
	}

	public struct GbApuState
	{
		[MarshalAs(UnmanagedType.I1)] public bool ApuEnabled;

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

		[MarshalAs(UnmanagedType.I1)] public bool ExtAudioLeftEnabled;
		[MarshalAs(UnmanagedType.I1)] public bool ExtAudioRightEnabled;

		public byte FrameSequenceStep;
	}

	public struct GbApuDebugState
	{
		public GbApuState Common;
		public GbSquareState Square1;
		public GbSquareState Square2;
		public GbWaveState Wave;
		public GbNoiseState Noise;
	}

	public struct NesCpuState : BaseState
	{
		public UInt64 CycleCount;
		public UInt16 PC;
		public byte SP;
		public byte A;
		public byte X;
		public byte Y;
		public byte PS;
		public byte IRQFlag;
		[MarshalAs(UnmanagedType.I1)] public bool NMIFlag;
	};

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
		public DebugSa1State Sa1;
		public GsuState Gsu;
		public Cx4State Cx4;

		public SnesDmaControllerState Dma;
		public InternalRegisterState InternalRegs;
		public AluState Alu;
	}

	public struct EmptyPpuToolsState : BaseState
	{
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

	public struct NesPpuStatusFlags
	{
		[MarshalAs(UnmanagedType.I1)] public bool SpriteOverflow;
		[MarshalAs(UnmanagedType.I1)] public bool Sprite0Hit;
		[MarshalAs(UnmanagedType.I1)] public bool VerticalBlank;
	}

	public struct NesPpuMaskFlags
	{
		[MarshalAs(UnmanagedType.I1)] public bool Grayscale;
		[MarshalAs(UnmanagedType.I1)] public bool BackgroundMask;
		[MarshalAs(UnmanagedType.I1)] public bool SpriteMask;
		[MarshalAs(UnmanagedType.I1)] public bool BackgroundEnabled;
		[MarshalAs(UnmanagedType.I1)] public bool SpritesEnabled;
		[MarshalAs(UnmanagedType.I1)] public bool IntensifyRed;
		[MarshalAs(UnmanagedType.I1)] public bool IntensifyGreen;
		[MarshalAs(UnmanagedType.I1)] public bool IntensifyBlue;
	}

	public struct NesPpuControlFlags
	{
		public UInt16 BackgroundPatternAddr;
		public UInt16 SpritePatternAddr;
		[MarshalAs(UnmanagedType.I1)] public bool VerticalWrite;
		[MarshalAs(UnmanagedType.I1)] public bool LargeSprites;
		[MarshalAs(UnmanagedType.I1)] public bool SecondaryPpu;
		[MarshalAs(UnmanagedType.I1)] public bool NmiOnVerticalBlank;
	}

	public struct NesPpuState : BaseState
	{
		public NesPpuStatusFlags StatusFlags;
		public NesPpuMaskFlags Mask;
		public NesPpuControlFlags Control;
		public Int32 Scanline;
		public UInt32 Cycle;
		public UInt32 FrameCount;
		public UInt32 NmiScanline;
		public UInt32 ScanlineCount;
		public UInt32 SafeOamScanline;
		public UInt16 BusAddress;
		public byte MemoryReadBuffer;

		public UInt16 VideoRamAddr;
		public UInt16 TmpVideoRamAddr;
		public byte ScrollX;
		[MarshalAs(UnmanagedType.I1)] public bool WriteToggle;
		public byte SpriteRamAddr;
	};

	[Flags]
	public enum NesCpuFlags
	{
		Carry = 0x01,
		Zero = 0x02,
		IrqDisable = 0x04,
		Decimal = 0x08,
		Overflow = 0x40,
		Negative = 0x80
	}

	[Flags]
	public enum NesIrqSources
	{
		External = 1,
		FrameCounter = 2,
		DMC = 4,
		FdsDisk = 8,
	}

	public struct NesState : BaseState
	{
		public NesCpuState Cpu;
		public NesPpuState Ppu;
		public NesCartridgeState Cartridge;
		public NesApuState Apu;
		public UInt32 ClockRate;
	}

	public enum NesPrgMemoryType
	{
		PrgRom,
		SaveRam,
		WorkRam,
	}

	public enum NesChrMemoryType
	{
		Default,
		ChrRom,
		ChrRam,
		NametableRam
	}

	public enum NesMemoryAccessType
	{
		Unspecified = -1,
		NoAccess = 0x00,
		Read = 0x01,
		Write = 0x02,
		ReadWrite = 0x03
	}

	public enum MapperStateValueType
	{
		None,
		String,
		Bool,
		Number8,
		Number16,
		Number32
	}

	public struct MapperStateEntry
	{
		public Int64 RawValue;
		public MapperStateValueType Type;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 40)] public byte[] Address;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 40)] public byte[] Name;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 40)] public byte[] Value;

		public string GetAddress() { return ConvertString(Address); }
		public string GetName() { return ConvertString(Name); }

		public object? GetValue()
		{
			switch(Type) {
				case MapperStateValueType.None: return null;

				case MapperStateValueType.String: return ConvertString(Value);
				case MapperStateValueType.Bool: return Value[0] != 0;
				
				case MapperStateValueType.Number8:
				case MapperStateValueType.Number16:
				case MapperStateValueType.Number32:
					Int64 value = 0;
					for(int i = 0; i < 8; i++) {
						value |= (Int64)Value[i] << (i * 8);
					}
					return value;
			}

			throw new Exception("Invalid value type");
		}

		private string ConvertString(byte[] stringArray)
		{
			int length = 0;
			for(int i = 0; i < 40; i++) {
				if(stringArray[i] == 0) {
					length = i;
					break;
				}
			}
			return Encoding.UTF8.GetString(stringArray, 0, length);
		}
	}

	public struct NesCartridgeState
	{
		public UInt32 PrgRomSize;
		public UInt32 ChrRomSize;
		public UInt32 ChrRamSize;

		public UInt32 PrgPageCount;
		public UInt32 PrgPageSize;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x100)]
		public Int32[] PrgMemoryOffset;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x100)]
		public NesPrgMemoryType[] PrgMemoryType;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x100)]
		public NesMemoryAccessType[] PrgMemoryAccess;

		public UInt32 ChrPageCount;
		public UInt32 ChrPageSize;
		public UInt32 ChrRamPageSize;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x40)]
		public Int32[] ChrMemoryOffset;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x40)]
		public NesChrMemoryType[] ChrMemoryType;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x40)]
		public NesMemoryAccessType[] ChrMemoryAccess;

		public UInt32 WorkRamPageSize;
		public UInt32 SaveRamPageSize;

		public NesMirroringType Mirroring;

		[MarshalAs(UnmanagedType.I1)]
		public bool HasBattery;

		public UInt32 CustomEntryCount;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 100)]
		public MapperStateEntry[] CustomEntries;
	}

	public enum NesMirroringType
	{
		Horizontal,
		Vertical,
		ScreenAOnly,
		ScreenBOnly,
		FourScreens
	}

	public struct NesApuLengthCounterState
	{
		[MarshalAs(UnmanagedType.I1)]
		public bool Halt;
		public byte Counter;
		public byte ReloadValue;
	}

	public struct NesApuEnvelopeState
	{
		[MarshalAs(UnmanagedType.I1)]
		public bool StartFlag;
		[MarshalAs(UnmanagedType.I1)]
		public bool Loop;
		[MarshalAs(UnmanagedType.I1)]
		public bool ConstantVolume;
		public byte Divider;
		public byte Counter;
		public byte Volume;
	}

	public struct NesApuSquareState
	{
		public byte Duty;
		public byte DutyPosition;
		public UInt16 Period;
		public UInt16 Timer;

		[MarshalAs(UnmanagedType.I1)]
		public bool SweepEnabled;
		[MarshalAs(UnmanagedType.I1)]
		public bool SweepNegate;
		public byte SweepPeriod;
		public byte SweepShift;

		[MarshalAs(UnmanagedType.I1)]
		public bool Enabled;
		public byte OutputVolume;
		public double Frequency;

		public NesApuLengthCounterState LengthCounter;
		public NesApuEnvelopeState Envelope;
	}

	public struct NesApuTriangleState
	{
		public UInt16 Period;
		public UInt16 Timer;
		public byte SequencePosition;

		[MarshalAs(UnmanagedType.I1)] public bool Enabled;
		public double Frequency;
		public byte OutputVolume;

		public byte LinearCounter;
		public byte LinearCounterReload;
		[MarshalAs(UnmanagedType.I1)] public bool LinearReloadFlag;

		public NesApuLengthCounterState LengthCounter;
	}

	public struct NesApuNoiseState
	{
		public UInt16 Period;
		public UInt16 Timer;
		public UInt16 ShiftRegister;
		[MarshalAs(UnmanagedType.I1)]
		public bool ModeFlag;

		[MarshalAs(UnmanagedType.I1)]
		public bool Enabled;
		public double Frequency;
		public byte OutputVolume;

		public NesApuLengthCounterState LengthCounter;
		public NesApuEnvelopeState Envelope;
	}

	public struct NesApuDmcState
	{
		public double SampleRate;
		public UInt16 SampleAddr;
		public UInt16 NextSampleAddr;
		public UInt16 SampleLength;

		[MarshalAs(UnmanagedType.I1)]
		public bool Loop;
		[MarshalAs(UnmanagedType.I1)]
		public bool IrqEnabled;
		public UInt16 Period;
		public UInt16 Timer;
		public UInt16 BytesRemaining;

		public byte OutputVolume;
	}

	public struct NesApuFrameCounterState
	{
		[MarshalAs(UnmanagedType.I1)]
		public bool FiveStepMode;
		public byte SequencePosition;
		[MarshalAs(UnmanagedType.I1)]
		public bool IrqEnabled;
	}

	public struct NesApuState
	{
		public NesApuSquareState Square1;
		public NesApuSquareState Square2;
		public NesApuTriangleState Triangle;
		public NesApuNoiseState Noise;
		public NesApuDmcState Dmc;
		public NesApuFrameCounterState FrameCounter;
	}

	public struct PceCpuState : BaseState
	{
		public UInt64 CycleCount;
		public UInt16 PC;
		public byte SP;
		public byte A;
		public byte X;
		public byte Y;
		public byte PS;
	}

	public enum PceCpuFlags
	{
		Carry = 0x01,
		Zero = 0x02,
		IrqDisable = 0x04,
		Decimal = 0x08,
		Memory = 0x20,
		Overflow = 0x40,
		Negative = 0x80
	}

	public struct PceVdcState : BaseState
	{
		public UInt32 FrameCount;
		
		public UInt16 HClock;
		public UInt16 Scanline;
		public UInt16 RcrCounter;

		public byte CurrentReg;

		//R00 - MAWR
		public UInt16 MemAddrWrite;

		//R01 - MARR
		public UInt16 MemAddrRead;
		public UInt16 ReadBuffer;

		//R02 - VWR
		public UInt16 VramData;

		//R05 - CR - Control
		[MarshalAs(UnmanagedType.I1)] public bool EnableCollisionIrq;
		[MarshalAs(UnmanagedType.I1)] public bool EnableOverflowIrq;
		[MarshalAs(UnmanagedType.I1)] public bool EnableScanlineIrq;
		[MarshalAs(UnmanagedType.I1)] public bool EnableVerticalBlankIrq;
		[MarshalAs(UnmanagedType.I1)] public bool OutputVerticalSync;
		[MarshalAs(UnmanagedType.I1)] public bool OutputHorizontalSync;
		[MarshalAs(UnmanagedType.I1)] public bool SpritesEnabled;
		[MarshalAs(UnmanagedType.I1)] public bool BackgroundEnabled;
		public byte VramAddrIncrement;

		//R06 - RCR
		public UInt16 RasterCompareRegister;

		//R09 - MWR - Memory Width
		public byte ColumnCount;
		public byte RowCount;
		public byte SpriteAccessMode;
		public byte VramAccessMode;
		[MarshalAs(UnmanagedType.I1)] public bool CgMode;

		[MarshalAs(UnmanagedType.I1)] public bool BgScrollYUpdatePending;

		public PceVdcHvLatches HvLatch;
		public PceVdcHvLatches HvReg;

		//R0F - DCR
		[MarshalAs(UnmanagedType.I1)] public bool VramSatbIrqEnabled;
		[MarshalAs(UnmanagedType.I1)] public bool VramVramIrqEnabled;
		[MarshalAs(UnmanagedType.I1)] public bool DecrementSrc;
		[MarshalAs(UnmanagedType.I1)] public bool DecrementDst;
		[MarshalAs(UnmanagedType.I1)] public bool RepeatSatbTransfer;

		//R10 - SOUR
		public UInt16 BlockSrc;

		//R11 - DESR
		public UInt16 BlockDst;

		//R12 - LENR
		public UInt16 BlockLen;

		//R13 - DVSSR
		public UInt16 SatbBlockSrc;
		[MarshalAs(UnmanagedType.I1)] public bool SatbTransferPending;
		[MarshalAs(UnmanagedType.I1)] public bool SatbTransferRunning;
		public UInt16 SatbTransferNextWordCounter;
		public byte SatbTransferOffset;

		//Status flags
		[MarshalAs(UnmanagedType.I1)] public bool VerticalBlank;
		[MarshalAs(UnmanagedType.I1)] public bool VramTransferDone;
		[MarshalAs(UnmanagedType.I1)] public bool SatbTransferDone;
		[MarshalAs(UnmanagedType.I1)] public bool ScanlineDetected;
		[MarshalAs(UnmanagedType.I1)] public bool SpriteOverflow;
		[MarshalAs(UnmanagedType.I1)] public bool Sprite0Hit;

		[MarshalAs(UnmanagedType.I1)] public bool BurstModeEnabled;
		[MarshalAs(UnmanagedType.I1)] public bool NextSpritesEnabled;
		[MarshalAs(UnmanagedType.I1)] public bool NextBackgroundEnabled;
	}

	public struct PceVceState : BaseState
	{
		public UInt16 ScanlineCount;
		public UInt16 PalAddr;
		public byte ClockDivider;
		[MarshalAs(UnmanagedType.I1)] public bool Grayscale;
	}

	public struct PceVdcHvLatches
	{
		//R07 - BXR
		public UInt16 BgScrollX;

		//R08 - BYR
		public UInt16 BgScrollY;

		//R0A - HSR
		public byte HorizDisplayStart;
		public byte HorizSyncWidth;

		//R0B - HDR
		public byte HorizDisplayWidth;
		public byte HorizDisplayEnd;

		//R0C - VPR
		public byte VertDisplayStart;
		public byte VertSyncWidth;

		//R0D - VDW
		public UInt16 VertDisplayWidth;

		//R0E - VCR
		public byte VertEndPosVcr;
	}

	public struct PceMemoryManagerState : BaseState
	{
		public UInt64 CycleCount;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
		public byte[] Mpr;

		public byte ActiveIrqs;
		public byte DisabledIrqs;
		[MarshalAs(UnmanagedType.I1)] public bool FastCpuSpeed;
		public byte MprReadBuffer;
		public byte IoBuffer;
	}

	public struct PceTimerState
	{
		public byte ReloadValue;
		public byte Counter;
		public UInt16 Scaler;
		[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	}

	public struct PcePsgState
	{
		public byte ChannelSelect;
		public byte LeftVolume;
		public byte RightVolume;
		public byte LfoFrequency;
		public byte LfoControl;
	};

	public struct PcePsgChannelState
	{
		public UInt16 Frequency;
		public byte Amplitude;
		[MarshalAs(UnmanagedType.I1)] public bool Enabled;
		public byte LeftVolume;
		public byte RightVolume;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x20)]
		public byte[] WaveData;

		[MarshalAs(UnmanagedType.I1)] public bool DdaEnabled;
		public byte DdaOutputValue;

		public byte WriteAddr;
		public byte ReadAddr;
		public UInt32 Timer;
		public byte CurrentOutput;

		//Channel 5 & 6 only
		public UInt32 NoiseLfsr;
		public UInt32 NoiseTimer;
		[MarshalAs(UnmanagedType.I1)] public bool NoiseEnabled;
		public sbyte NoiseOutput;
		public byte NoiseFrequency;
	}

	public enum PceVpcPriorityMode
	{
		Default = 0,
		Vdc1SpritesBelowVdc2Bg = 1,
		Vdc2SpritesAboveVdc1Bg = 2,
	}

	public enum PceVpcPixelWindow
	{
		NoWindow,
		Window1,
		Window2,
		Both
	}

	public struct PceVpcPriorityConfig
	{
		public PceVpcPriorityMode PriorityMode;
		[MarshalAs(UnmanagedType.I1)] public bool Vdc1Enabled;
		[MarshalAs(UnmanagedType.I1)] public bool Vdc2Enabled;
	}

	public struct PceVpcState
	{
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
		public PceVpcPriorityConfig[] WindowCfg;
		public byte Priority1;
		public byte Priority2;
		public UInt16 Window1;
		public UInt16 Window2;
		[MarshalAs(UnmanagedType.I1)] public bool StToVdc2Mode;
		[MarshalAs(UnmanagedType.I1)] public bool HasIrqVdc1;
		[MarshalAs(UnmanagedType.I1)] public bool HasIrqVdc2;
	}

	public struct PceVideoState : BaseState
	{
		public PceVdcState Vdc;
		public PceVceState Vce;
		public PceVpcState Vpc;
		public PceVdcState Vdc2;
	}

	public enum PceArcadePortOffsetTrigger
	{
		None = 0,
		AddOnLowWrite = 1,
		AddOnHighWrite = 2,
		AddOnReg0AWrite = 3,
	}

	public struct PceArcadeCardPortConfig
	{
		public UInt32 BaseAddress;
		public UInt16 Offset;
		public UInt16 IncValue;

		public byte Control;
		[MarshalAs(UnmanagedType.I1)] public bool AutoIncrement;
		[MarshalAs(UnmanagedType.I1)] public bool AddOffset;
		[MarshalAs(UnmanagedType.I1)] public bool SignedIncrement; //unused?
		[MarshalAs(UnmanagedType.I1)] public bool SignedOffset;
		[MarshalAs(UnmanagedType.I1)] public bool AddIncrementToBase;
		public PceArcadePortOffsetTrigger AddOffsetTrigger;
	}

	public struct PceArcadeCardState
	{
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
		public PceArcadeCardPortConfig[] Port;
		public UInt32 ValueReg;
		public byte ShiftReg;
		public byte RotateReg;
	}

	public enum PceCdRomIrqSource
	{
		Adpcm = 0x04,
		Stop = 0x08,
		SubChannel = 0x10,
		DataTransferDone = 0x20,
		DataTransferReady = 0x40
	}

	public struct PceCdRomState
	{
		public byte ActiveIrqs;
		public byte EnabledIrqs;
		[MarshalAs(UnmanagedType.I1)] public bool ReadRightChannel;
		[MarshalAs(UnmanagedType.I1)] public bool BramLocked;
	}

	public struct PceAdpcmState
	{
		[MarshalAs(UnmanagedType.I1)] public bool Nibble;
		public UInt16 ReadAddress;
		public UInt16 WriteAddress;

		public UInt16 AddressPort;

		public byte DmaControl;
		public byte Control;
		public byte PlaybackRate;

		public UInt16 AdpcmLength;
		[MarshalAs(UnmanagedType.I1)] public bool EndReached;
		[MarshalAs(UnmanagedType.I1)] public bool HalfReached;

		[MarshalAs(UnmanagedType.I1)] public bool Playing;

		public byte ReadBuffer;
		public byte ReadClockCounter;

		public byte WriteBuffer;
		public byte WriteClockCounter;
	}

	public enum CdPlayEndBehavior
	{
		Stop,
		Loop,
		Irq
	}

	public enum CdAudioStatus : byte
	{
		Playing = 0,
		Inactive = 1,
		Paused = 2,
		Stopped = 3
	}

	public struct PceCdAudioPlayerState
	{
		public CdAudioStatus Status;

		public UInt32 StartSector;
		public UInt32 EndSector;
		public CdPlayEndBehavior EndBehavior;

		public UInt32 CurrentSector;
		public UInt32 CurrentSample;

		public Int16 LeftSample;
		public Int16 RightSample;
	}

	public enum ScsiPhase
	{
		BusFree,
		Command,
		DataIn,
		DataOut,
		MessageIn,
		MessageOut,
		Status
	}

	public struct PceScsiBusState
	{
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 9)]
		public byte[] Signals;

		public ScsiPhase Phase;

		[MarshalAs(UnmanagedType.I1)] public bool StatusDone;
		[MarshalAs(UnmanagedType.I1)] public bool MessageDone;
		public byte MessageData;
		public byte DataPort;

		[MarshalAs(UnmanagedType.I1)] public bool DiscReading;
		[MarshalAs(UnmanagedType.I1)] public bool DataTransfer;
		[MarshalAs(UnmanagedType.I1)] public bool DataTransferDone;

		public UInt32 Sector;
		public byte SectorsToRead;
	}

	public enum PceAudioFaderTarget
	{
		Adpcm,
		CdAudio,
	}

	public struct PceAudioFaderState
	{
		public UInt64 StartClock;
		public PceAudioFaderTarget Target;
		[MarshalAs(UnmanagedType.I1)] public bool FastFade;
		[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	}

	public struct PceState : BaseState
	{
		public PceCpuState Cpu;
		public PceVideoState Video;

		public PceMemoryManagerState MemoryManager;
		public PceTimerState Timer;

		public PcePsgState Psg;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 6)]
		public PcePsgChannelState[] PsgChannels;

		public PceCdRomState CdRom;
		public PceCdAudioPlayerState CdPlayer;
		public PceAdpcmState Adpcm;
		public PceAudioFaderState AudioFader;
		public PceScsiBusState ScsiDrive;
		public PceArcadeCardState ArcadeCard;

		[MarshalAs(UnmanagedType.I1)] public bool IsSuperGrafx;
		[MarshalAs(UnmanagedType.I1)] public bool HasArcadeCard;
		[MarshalAs(UnmanagedType.I1)] public bool HasCdRom;
	}
}
