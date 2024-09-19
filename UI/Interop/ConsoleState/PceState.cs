using System;
using System.Runtime.InteropServices;

namespace Mesen.Interop;

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

	//R09 - MWR - Memory Width
	public byte ColumnCount;
	public byte RowCount;
	public byte SpriteAccessMode;
	public byte VramAccessMode;
	[MarshalAs(UnmanagedType.I1)] public bool CgMode;

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
	DataTransferDone = 0x20,
	DataTransferReady = 0x40
}

public struct PceCdRomState
{
	public UInt16 AudioSampleLatch;
	public byte ActiveIrqs;
	public byte EnabledIrqs;
	[MarshalAs(UnmanagedType.I1)] public bool ReadRightChannel;
	[MarshalAs(UnmanagedType.I1)] public bool BramLocked;
	public byte ResetRegValue;
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

	public UInt32 AdpcmLength;
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

	[MarshalAs(UnmanagedType.I1)] public bool MessageDone;
	public byte DataPort;
	public byte ReadDataPort;

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
	public byte RegValue;
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
