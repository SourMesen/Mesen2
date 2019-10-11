using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
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
	};

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
		public byte IrqSource;
		public byte PrevIrqSource;
		public CpuStopState StopState;
	};

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

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5, ArraySubType = UnmanagedType.I1)]
		public bool[] WindowMaskMain;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5, ArraySubType = UnmanagedType.I1)]
		public bool[] WindowMaskSub;

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
	};

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
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 6, ArraySubType = UnmanagedType.I1)]
		public bool[] ActiveLayers;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 6, ArraySubType = UnmanagedType.I1)]
		public bool[] InvertedLayers;

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
	};

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
	};

	public struct GsuPixelCache
	{
		public byte X;
		public byte Y;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
		public byte[] Pixels;
		public byte ValidBits;
	};

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
	};

	public struct Cx4Dma
	{
		public UInt32 Source;
		public UInt32 Dest;
		public UInt16 Length;
		public UInt32 Pos;
		[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	};

	public struct Cx4Suspend
	{
		public UInt32 Duration;
		[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	};

	public struct Cx4Cache
	{
		[MarshalAs(UnmanagedType.I1)] public bool Enabled;
		public byte Page;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2, ArraySubType = UnmanagedType.I1)]
		public bool[] Lock;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
		public UInt32[] Address;

		public UInt32 Base;
		public UInt16 ProgramBank;
		public byte ProgramCounter;
		public UInt16 Pos;
	};

	public struct Cx4Bus
	{
		[MarshalAs(UnmanagedType.I1)] public bool Enabled;
		[MarshalAs(UnmanagedType.I1)] public bool Reading;
		[MarshalAs(UnmanagedType.I1)] public bool Writing;
		public byte DelayCycles;
		public UInt32 Address;
	};

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
	};

	public struct AluState
	{
		public byte MultOperand1;
		public byte MultOperand2;
		public UInt16 MultOrRemainderResult;

		public UInt16 Dividend;
		public byte Divisor;
		public UInt16 DivResult;
	};

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
	};

	public struct DebugState
	{
		public UInt64 MasterClock;
		public CpuState Cpu;
		public PpuState Ppu;
		public SpcState Spc;
		public NecDspState NecDsp;
		public CpuState Sa1;
		public GsuState Gsu;
		public Cx4State Cx4;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
		public DmaChannelConfig[] DmaChannels;

		public InternalRegisterState InternalRegs;
		public AluState Alu;
	}
}
