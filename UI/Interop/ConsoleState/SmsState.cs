using System;
using System.Runtime.InteropServices;

namespace Mesen.Interop;

public struct SmsCpuState : BaseState
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

	public byte IXL;
	public byte IXH;

	public byte IYL;
	public byte IYH;

	public byte I;
	public byte R;

	public byte AltA;
	public byte AltFlags;
	public byte AltB;
	public byte AltC;
	public byte AltD;
	public byte AltE;
	public byte AltH;
	public byte AltL;

	public byte ActiveIrqs;
	[MarshalAs(UnmanagedType.I1)] public bool NmiLevel;
	[MarshalAs(UnmanagedType.I1)] public bool NmiPending;
	[MarshalAs(UnmanagedType.I1)] public bool Halted;

	[MarshalAs(UnmanagedType.I1)] public bool IFF1;
	[MarshalAs(UnmanagedType.I1)] public bool IFF2;
	public byte IM;

	public byte FlagsChanged;
	public UInt16 WZ;
}

[Flags]
public enum SmsCpuFlags : byte
{
	Carry = 0x01,
	AddSub = 0x02,
	Parity = 0x04,
	F3 = 0x08,
	HalfCarry = 0x10,
	F5 = 0x20,
	Zero = 0x40,
	Sign = 0x80,
}

public struct SmsVdpState : BaseState
{
	public UInt32 FrameCount;
	public UInt16 Cycle;
	public UInt16 Scanline;
	public UInt16 VCounter;

	public UInt16 AddressReg;
	public byte CodeReg;
	[MarshalAs(UnmanagedType.I1)] public bool ControlPortMsbToggle;

	public byte ReadBuffer;
	public byte PaletteLatch;
	public byte HCounterLatch;

	[MarshalAs(UnmanagedType.I1)] public bool VerticalBlankIrqPending;
	[MarshalAs(UnmanagedType.I1)] public bool ScanlineIrqPending;
	[MarshalAs(UnmanagedType.I1)] public bool SpriteOverflow;
	[MarshalAs(UnmanagedType.I1)] public bool SpriteCollision;
	public byte SpriteOverflowIndex;

	public UInt16 ColorTableAddress;
	public UInt16 BgPatternTableAddress;

	public UInt16 SpriteTableAddress;
	public UInt16 SpritePatternSelector;

	public UInt16 NametableHeight;
	public byte VisibleScanlineCount;

	public byte TextColorIndex;
	public byte BackgroundColorIndex;
	public byte HorizontalScroll;
	public byte HorizontalScrollLatch;
	public byte VerticalScroll;
	public byte VerticalScrollLatch;
	public byte ScanlineCounter;
	public byte ScanlineCounterLatch;

	[MarshalAs(UnmanagedType.I1)] public bool SyncDisabled;
	[MarshalAs(UnmanagedType.I1)] public bool M2_AllowHeightChange;
	[MarshalAs(UnmanagedType.I1)] public bool UseMode4;
	[MarshalAs(UnmanagedType.I1)] public bool ShiftSpritesLeft;
	[MarshalAs(UnmanagedType.I1)] public bool EnableScanlineIrq;
	[MarshalAs(UnmanagedType.I1)] public bool MaskFirstColumn;
	[MarshalAs(UnmanagedType.I1)] public bool HorizontalScrollLock;
	[MarshalAs(UnmanagedType.I1)] public bool VerticalScrollLock;

	[MarshalAs(UnmanagedType.I1)] public bool Sg16KVramMode;
	[MarshalAs(UnmanagedType.I1)] public bool RenderingEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool EnableVerticalBlankIrq;
	[MarshalAs(UnmanagedType.I1)] public bool M1_Use224LineMode;
	[MarshalAs(UnmanagedType.I1)] public bool M3_Use240LineMode;
	[MarshalAs(UnmanagedType.I1)] public bool UseLargeSprites;
	[MarshalAs(UnmanagedType.I1)] public bool EnableDoubleSpriteSize;

	public UInt16 NametableAddress;
	public UInt16 EffectiveNametableAddress;
	public UInt16 NametableAddressMask;
	public UInt16 EffectiveNametableAddressMask;
}

public struct SmsToneChannelState
{
	public UInt16 ReloadValue;
	public UInt16 Timer;
	public byte Output;
	public byte Volume;
}

public struct SmsNoiseChannelState
{
	public UInt16 Timer;
	public UInt16 Lfsr;
	public byte LfsrInputBit;
	public byte Control;
	public byte Output;
	public byte Volume;
}

public struct SmsPsgState
{
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 3)]
	public SmsToneChannelState[] Tone;
	public SmsNoiseChannelState Noise;
	public byte SelectedReg;
	public byte GameGearPanningReg;
}

public enum SmsRegisterAccess
{
	None = 0,
	Read = 1,
	Write = 2,
	ReadWrite = 3
}

public struct SmsMemoryManagerState
{
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x100)]
	public byte[] IsReadRegister;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x100)]
	public byte[] IsWriteRegister;

	public byte OpenBus;
	public byte GgExtData;
	public byte GgExtConfig;
	public byte GgSendData;
	public byte GgSerialConfig;

	[MarshalAs(UnmanagedType.I1)] public bool ExpEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool CartridgeEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool CardEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool WorkRamEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool BiosEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool IoEnabled;
}

public struct SmsControlManagerState
{
	public byte ControlPort;
}

public struct SmsState : BaseState
{
	public SmsCpuState Cpu;
	public SmsVdpState Vdp;
	public SmsPsgState Psg;
	public SmsMemoryManagerState MemoryManager;
	public SmsControlManagerState ControlManager;
}
