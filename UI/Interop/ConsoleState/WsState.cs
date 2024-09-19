using Mesen.Config;
using System;
using System.Runtime.InteropServices;

namespace Mesen.Interop;

public struct WsBgLayer
{
	public UInt16 MapAddress;
	public UInt16 MapAddressLatch;
	public byte ScrollX;
	public byte ScrollXLatch;
	public byte ScrollY;
	public byte ScrollYLatch;
	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	[MarshalAs(UnmanagedType.I1)] public bool EnabledLatch;
}

public struct WsWindow
{
	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	[MarshalAs(UnmanagedType.I1)] public bool EnabledLatch;
	public byte Left;
	public byte LeftLatch;
	public byte Right;
	public byte RightLatch;
	public byte Top;
	public byte TopLatch;
	public byte Bottom;
	public byte BottomLatch;
}

public struct WsLcdIcons
{
	[MarshalAs(UnmanagedType.I1)] public bool Sleep;
	[MarshalAs(UnmanagedType.I1)] public bool Vertical;
	[MarshalAs(UnmanagedType.I1)] public bool Horizontal;
	[MarshalAs(UnmanagedType.I1)] public bool Aux1;
	[MarshalAs(UnmanagedType.I1)] public bool Aux2;
	[MarshalAs(UnmanagedType.I1)] public bool Aux3;

	public byte Value;
}

public enum WsVideoMode : byte
{
	Monochrome,
	Color2bpp,
	Color4bpp,
	Color4bppPacked
}

public static class WsVideoModeExtensions
{
	public static TileFormat ToTileFormat(this WsVideoMode mode)
	{
		return mode switch {
			WsVideoMode.Monochrome => TileFormat.Bpp2,
			WsVideoMode.Color2bpp => TileFormat.Bpp2,
			WsVideoMode.Color4bpp => TileFormat.SmsBpp4,
			WsVideoMode.Color4bppPacked or _ => TileFormat.WsBpp4Packed
		};
	}
}

public struct WsPpuState : BaseState
{
	public UInt32 FrameCount;
	public UInt16 Cycle;
	public UInt16 Scanline;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
	public WsBgLayer[] BgLayers;

	public WsWindow BgWindow;
	public WsWindow SpriteWindow;
	[MarshalAs(UnmanagedType.I1)] public bool DrawOutsideBgWindow;
	[MarshalAs(UnmanagedType.I1)] public bool DrawOutsideBgWindowLatch;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x40)]
	public byte[] BwPalettes;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
	public byte[] BwShades;

	public UInt16 SpriteTableAddress;
	public byte FirstSpriteIndex;
	public byte SpriteCount;
	public byte SpriteCountLatch;
	[MarshalAs(UnmanagedType.I1)] public bool SpritesEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool SpritesEnabledLatch;

	public WsVideoMode Mode;
	public WsVideoMode NextMode;

	public byte BgColor;
	public byte IrqScanline;

	[MarshalAs(UnmanagedType.I1)] public bool LcdEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool HighContrast;
	[MarshalAs(UnmanagedType.I1)] public bool SleepEnabled;

	public WsLcdIcons Icons;

	public byte LastScanline;
	public byte BackPorchScanline;

	public UInt32 ShowVolumeIconFrame;

	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8)]
	public byte[] LcdTftConfig;

	public byte Control;
	public byte ScreenAddress;
}

public struct WsCpuFlags
{
	[MarshalAs(UnmanagedType.I1)] public bool Carry;
	[MarshalAs(UnmanagedType.I1)] public bool Parity;
	[MarshalAs(UnmanagedType.I1)] public bool AuxCarry;
	[MarshalAs(UnmanagedType.I1)] public bool Zero;
	[MarshalAs(UnmanagedType.I1)] public bool Sign;
	[MarshalAs(UnmanagedType.I1)] public bool Trap;
	[MarshalAs(UnmanagedType.I1)] public bool Irq;
	[MarshalAs(UnmanagedType.I1)] public bool Direction;
	[MarshalAs(UnmanagedType.I1)] public bool Overflow;
	[MarshalAs(UnmanagedType.I1)] public bool Mode;

	public UInt16 Get()
	{
		return (UInt16)(
			(Carry ? 0x01 : 0) |
			(Parity ? 0x04 : 0) |
			(AuxCarry ? 0x10 : 0) |
			(Zero ? 0x40 : 0) |
			(Sign ? 0x80 : 0) |
			(Trap ? 0x100 : 0) |
			(Irq ? 0x200 : 0) |
			(Direction ? 0x400 : 0) |
			(Overflow ? 0x800 : 0) |
			(Mode ? 0x8000 : 0) |
			0x7002
		);
	}

	void Set(UInt16 f)
	{
		Carry = (f & 0x01) != 0;
		Parity = (f & 0x04) != 0;
		AuxCarry = (f & 0x10) != 0;
		Zero = (f & 0x40) != 0;
		Sign = (f & 0x80) != 0;
		Trap = (f & 0x100) != 0;
		Irq = (f & 0x200) != 0;
		Direction = (f & 0x400) != 0;
		Overflow = (f & 0x800) != 0;
		Mode = (f & 0x8000) != 0;
	}
}

public struct WsCpuState : BaseState
{
	public UInt64 CycleCount;

	public UInt16 CS;
	public UInt16 IP;

	public UInt16 SS;
	public UInt16 SP;
	public UInt16 BP;

	public UInt16 DS;
	public UInt16 ES;

	public UInt16 SI;
	public UInt16 DI;

	public UInt16 AX;
	public UInt16 BX;
	public UInt16 CX;
	public UInt16 DX;

	public WsCpuFlags Flags;
	[MarshalAs(UnmanagedType.I1)] public bool Halted;
}

public enum WsIrqSource : byte
{
	UartSendReady = 0x01,
	KeyPressed = 0x02,
	Cart = 0x04,
	UartRecvReady = 0x08,
	Scanline = 0x10,
	VerticalBlankTimer = 0x20,
	VerticalBlank = 0x40,
	HorizontalBlankTimer = 0x80
}

public struct WsMemoryManagerState
{
	public byte ActiveIrqs;
	public byte EnabledIrqs;
	public byte IrqVectorOffset;

	public byte SystemControl2;
	[MarshalAs(UnmanagedType.I1)] public bool ColorEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool Enable4bpp;
	[MarshalAs(UnmanagedType.I1)] public bool Enable4bppPacked;
	
	[MarshalAs(UnmanagedType.I1)] public bool BootRomDisabled;
	[MarshalAs(UnmanagedType.I1)] public bool CartWordBus;
	[MarshalAs(UnmanagedType.I1)] public bool SlowRom;
	
	[MarshalAs(UnmanagedType.I1)] public bool SlowSram;
	[MarshalAs(UnmanagedType.I1)] public bool SlowPort;
	[MarshalAs(UnmanagedType.I1)] public bool EnableLowBatteryNmi;
}

public struct WsControlManagerState
{
	public byte InputSelect;
}

public struct WsDmaControllerState
{
	public UInt32 GdmaSrc;
	public UInt32 SdmaSrc;
	public UInt32 SdmaLength;
	public UInt32 SdmaSrcReloadValue;
	public UInt32 SdmaLengthReloadValue;

	public UInt16 GdmaDest;
	public UInt16 GdmaLength;
	public byte GdmaControl;
	public byte SdmaControl;

	[MarshalAs(UnmanagedType.I1)] public bool SdmaEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool SdmaDecrement;
	[MarshalAs(UnmanagedType.I1)] public bool SdmaHyperVoice;
	[MarshalAs(UnmanagedType.I1)] public bool SdmaRepeat;
	[MarshalAs(UnmanagedType.I1)] public bool SdmaHold;
	public byte SdmaFrequency;
	public byte SdmaTimer;
}

public struct WsTimerState
{
	public UInt16 HTimer;
	public UInt16 VTimer;

	public UInt16 HReloadValue;
	public UInt16 VReloadValue;

	public byte Control;
	[MarshalAs(UnmanagedType.I1)] public bool HBlankEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool HBlankAutoReload;
	[MarshalAs(UnmanagedType.I1)] public bool VBlankEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool VBlankAutoReload;
}

public struct WsApuCh1State
{
	public UInt16 Frequency;
	public UInt16 Timer;

	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	public byte LeftVolume;
	public byte RightVolume;

	public byte SamplePosition;
	public byte LeftOutput;
	public byte RightOutput;
}

public struct WsApuCh2State
{
	public UInt16 Frequency;
	public UInt16 Timer;

	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	public byte LeftVolume;
	public byte RightVolume;

	public byte SamplePosition;
	public byte LeftOutput;
	public byte RightOutput;

	[MarshalAs(UnmanagedType.I1)] public bool PcmEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool MaxPcmVolumeRight;
	[MarshalAs(UnmanagedType.I1)] public bool HalfPcmVolumeRight;
	[MarshalAs(UnmanagedType.I1)] public bool MaxPcmVolumeLeft;
	[MarshalAs(UnmanagedType.I1)] public bool HalfPcmVolumeLeft;
};

public struct WsApuCh3State
{
	public UInt16 Frequency;
	public UInt16 Timer;

	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	public byte LeftVolume;
	public byte RightVolume;

	public byte SamplePosition;
	public byte LeftOutput;
	public byte RightOutput;

	public UInt16 SweepScaler;
	[MarshalAs(UnmanagedType.I1)] public bool SweepEnabled;
	public sbyte SweepValue;
	public byte SweepPeriod;
	public byte SweepTimer;
	[MarshalAs(UnmanagedType.I1)] public bool UseSweepCpuClock;
};

public struct WsApuCh4State
{
	public UInt16 Frequency;
	public UInt16 Timer;

	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	public byte LeftVolume;
	public byte RightVolume;

	public byte SamplePosition;
	public byte LeftOutput;
	public byte RightOutput;

	[MarshalAs(UnmanagedType.I1)] public bool NoiseEnabled;
	[MarshalAs(UnmanagedType.I1)] public bool LfsrEnabled;
	public byte TapMode;
	public byte TapShift;
	public UInt16 Lfsr;
	public byte HoldLfsr;
};

public enum WsHyperVoiceScalingMode : byte
{
	Unsigned,
	UnsignedNegated,
	Signed,
	None
}

public enum WsHyperVoiceChannelMode : byte
{
	Stereo,
	MonoLeft,
	MonoRight,
	MonoBoth
}

public struct WsApuHyperVoiceState
{
	public Int16 LeftOutput;
	public Int16 RightOutput;
	
	[MarshalAs(UnmanagedType.I1)] public bool Enabled;

	public byte LeftSample;
	public byte RightSample;
	[MarshalAs(UnmanagedType.I1)] public bool UpdateRightOutput;

	public byte Divisor;
	public byte Timer;
	public byte Input;
	public byte Shift;
	public WsHyperVoiceChannelMode ChannelMode;
	public WsHyperVoiceScalingMode ScalingMode;

	public byte ControlLow;
	public byte ControlHigh;
};

public struct WsApuState
{
	public WsApuCh1State Ch1;
	public WsApuCh2State Ch2;
	public WsApuCh3State Ch3;
	public WsApuCh4State Ch4;
	public WsApuHyperVoiceState Voice;

	public UInt16 WaveTableAddress;
	[MarshalAs(UnmanagedType.I1)] public bool SpeakerEnabled;
	public byte SpeakerVolume;
	public byte MasterVolume;
	public byte InternalMasterVolume;
	[MarshalAs(UnmanagedType.I1)] public bool HeadphoneEnabled;

	[MarshalAs(UnmanagedType.I1)] public bool HoldChannels;
	[MarshalAs(UnmanagedType.I1)] public bool ForceOutput2;
	[MarshalAs(UnmanagedType.I1)] public bool ForceOutput4;
	[MarshalAs(UnmanagedType.I1)] public bool ForceOutputCh2Voice;
};

public struct WsSerialState
{
	public UInt64 SendClock;

	[MarshalAs(UnmanagedType.I1)] public bool Enabled;
	[MarshalAs(UnmanagedType.I1)] public bool HighSpeed;
	[MarshalAs(UnmanagedType.I1)] public bool ReceiveOverflow;

	[MarshalAs(UnmanagedType.I1)] public bool HasReceiveData;
	public byte ReceiveBuffer;

	[MarshalAs(UnmanagedType.I1)] public bool HasSendData;
	public byte SendBuffer;
}

public enum WsEepromSize : UInt16
{
	Size0 = 0,
	Size128 = 0x80,
	Size1kb = 0x400,
	Size2kb = 0x800
}

public struct WsEepromState
{
	public WsEepromSize Size;
	public UInt16 ReadBuffer;
	public UInt16 WriteBuffer;
	public UInt16 Command;
	public UInt16 Control;
	[MarshalAs(UnmanagedType.I1)] public bool WriteDisabled;
	[MarshalAs(UnmanagedType.I1)] public bool ReadDone;
	[MarshalAs(UnmanagedType.I1)] public bool Idle;

	[MarshalAs(UnmanagedType.I1)] public bool InternalEepromWriteProtected;
}

public struct WsCartState
{
	[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
	public byte[] SelectedBanks;
}

public struct WsState : BaseState
{
	public WsCpuState Cpu;
	public WsPpuState Ppu;
	public WsApuState Apu;
	public WsMemoryManagerState MemoryManager;
	public WsControlManagerState ControlManager;
	public WsDmaControllerState DmaController;
	public WsTimerState Timer;
	public WsSerialState Serial;
	public WsEepromState InternalEeprom;
	public WsCartState Cart;
	public WsEepromState CartEeprom;
	public WsModel Model;
}