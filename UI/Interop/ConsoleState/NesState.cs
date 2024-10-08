using System;
using System.Runtime.InteropServices;
using System.Text;

namespace Mesen.Interop;

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
	MapperRam,
}

public enum NesChrMemoryType
{
	Default,
	ChrRom,
	ChrRam,
	NametableRam,
	MapperRam,
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
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 200)]
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

public struct NtExtConfig
{
	public UInt16 SourceOffset;
	[MarshalAs(UnmanagedType.I1)] public bool AttrExtMode;
	[MarshalAs(UnmanagedType.I1)] public bool BgExtMode;
	[MarshalAs(UnmanagedType.I1)] public bool FillMode;
};

public struct ExtModeConfig
{
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5)]
	public NtExtConfig[] Nametables;

	[MarshalAs(UnmanagedType.I1)] public bool SpriteExtMode;
	public byte BgExtBank;
	public byte SpriteExtBank;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 64)]
	public byte[] SpriteExtData;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x2000)]
	public byte[] ExtRam;
};

public struct NesPpuToolsState : BaseState
{
	public ExtModeConfig ExtConfig;
}