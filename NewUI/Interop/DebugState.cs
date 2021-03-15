using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace Mesen.GUI
{
	public enum CpuStopState : byte
	{
		Running = 0,
		Stopped = 1,
		WaitingForIrq = 2
	}

	[Flags]
	public enum ProcFlags : byte
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

	public struct CpuState
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
		public ProcFlags PS;
		[MarshalAs(UnmanagedType.I1)] public bool EmulationMode;

		[MarshalAs(UnmanagedType.I1)] public bool NmiFlag;
		[MarshalAs(UnmanagedType.I1)] public bool PrevNmiFlag;

		[MarshalAs(UnmanagedType.I1)] public bool IrqLock;
		[MarshalAs(UnmanagedType.I1)] public bool PrevNeedNmi;
		[MarshalAs(UnmanagedType.I1)] public bool NeedNmi;

		public byte IrqSource;
		public byte PrevIrqSource;
		public CpuStopState StopState;
	}

	public struct PpuState
	{
		public UInt16 Cycle;
		public UInt16 Scanline;
		public UInt16 HClock;
		public UInt32 FrameCount;

		[MarshalAs(UnmanagedType.I1)] public bool ForcedVblank;
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
		[MarshalAs(UnmanagedType.I1)] public bool ColorMathSubstractMode;
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

	public struct SpcState
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
		public CpuStopState StopState;

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

	public struct NecDspState
	{
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

	public struct GsuState
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

	public struct Cx4State
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
		public CpuState Cpu;
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
		PrgRom = (int)SnesMemoryType.GbPrgRom,
		WorkRam = (int)SnesMemoryType.GbWorkRam,
		CartRam = (int)SnesMemoryType.GbCartRam,
		BootRom = (int)SnesMemoryType.GbBootRom,
	}

	public enum RegisterAccess
	{
		None = 0,
		Read = 1,
		Write = 2,
		ReadWrite = 3
	}

	public struct GbMemoryManagerState
	{
		public UInt64 CycleCount;
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
		public byte InputSelect;

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
		public RegisterAccess[] MemoryAccessType;
	}

	public struct GbDmaControllerState
	{
		public byte OamDmaSource;
		public byte DmaStartDelay;
		public byte InternalDest;
		public byte DmaCounter;
		public byte DmaReadBuffer;

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

	public struct GbState
	{
		public GbType Type;
		public GbCpuState Cpu;
		public GbPpuState Ppu;
		public GbApuDebugState Apu;
		public GbMemoryManagerState MemoryManager;
		public GbTimerState Timer;
		public GbDmaControllerState Dma;
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

	public struct GbCpuState
	{
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

	public struct GbPpuState
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
		public UInt16 SweepPeriod;
		[MarshalAs(UnmanagedType.I1)] public bool SweepNegate;
		public byte SweepShift;

		public UInt16 SweepTimer;
		[MarshalAs(UnmanagedType.I1)] public bool SweepEnabled;
		public UInt16 SweepFreq;

		public byte Volume;
		public byte EnvVolume;
		[MarshalAs(UnmanagedType.I1)] public bool EnvRaiseVolume;
		public byte EnvPeriod;
		public byte EnvTimer;
		[MarshalAs(UnmanagedType.I1)] public bool EnvStopped;

		public byte Duty;
		public UInt16 Frequency;

		public byte Length;
		[MarshalAs(UnmanagedType.I1)] public bool LengthEnabled;

		[MarshalAs(UnmanagedType.I1)] public bool Enabled;
		public UInt16 Timer;
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

	public struct DebugState
	{
		public UInt64 MasterClock;
		public CpuState Cpu;
		public PpuState Ppu;
		public SpcState Spc;
		public DspState Dsp;
		public NecDspState NecDsp;
		public DebugSa1State Sa1;
		public GsuState Gsu;
		public Cx4State Cx4;
		
		public GbState Gameboy;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
		public DmaChannelConfig[] DmaChannels;

		public InternalRegisterState InternalRegs;
		public AluState Alu;
	}
}
