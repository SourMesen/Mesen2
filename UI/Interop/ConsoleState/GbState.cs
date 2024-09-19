using System;
using System.Runtime.InteropServices;

namespace Mesen.Interop;

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

	public byte CgbRegRpInfrared;

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
	public UInt16 HaltCounter;

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
	[MarshalAs(UnmanagedType.I1)] public bool HaltBug;
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

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4 * 8)]
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

public struct GbNoiseState
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
