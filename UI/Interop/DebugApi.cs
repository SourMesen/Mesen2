using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Config;
using Mesen.GUI.Debugger;
using Mesen.GUI.Forms;

namespace Mesen.GUI
{
	public class DebugApi
	{
		private const string DllPath = "MesenSCore.dll";
		[DllImport(DllPath)] public static extern void InitializeDebugger();
		[DllImport(DllPath)] public static extern void ReleaseDebugger();

		[DllImport(DllPath)] public static extern void ResumeExecution();
		[DllImport(DllPath)] public static extern void Step(Int32 instructionCount, StepType type = StepType.CpuStep);

		[DllImport(DllPath)] public static extern void StartTraceLogger([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string filename);
		[DllImport(DllPath)] public static extern void StopTraceLogger();
		[DllImport(DllPath)] public static extern void SetTraceOptions(InteropTraceLoggerOptions options);

		[DllImport(DllPath, EntryPoint = "GetDisassemblyLineData")] private static extern void GetDisassemblyLineDataWrapper(CpuType type, UInt32 lineIndex, ref InteropCodeLineData lineData);
		public static CodeLineData GetDisassemblyLineData(CpuType type, UInt32 lineIndex)
		{
			InteropCodeLineData data = new InteropCodeLineData();
			data.Comment = new byte[1000];
			data.Text = new byte[1000];
			data.ByteCode = new byte[4];

			DebugApi.GetDisassemblyLineDataWrapper(type, lineIndex, ref data);
			return new CodeLineData(data, type);
		}

		[DllImport(DllPath)] public static extern void RefreshDisassembly(CpuType type);
		[DllImport(DllPath)] public static extern UInt32 GetDisassemblyLineCount(CpuType type);
		[DllImport(DllPath)] public static extern UInt32 GetDisassemblyLineIndex(CpuType type, UInt32 cpuAddress);
		[DllImport(DllPath)] public static extern int SearchDisassembly(CpuType type, [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string searchString, int startPosition, int endPosition, [MarshalAs(UnmanagedType.I1)]bool searchBackwards);

		[DllImport(DllPath, EntryPoint = "GetExecutionTrace")] private static extern IntPtr GetExecutionTraceWrapper(UInt32 lineCount);
		public static string GetExecutionTrace(UInt32 lineCount) { return Utf8Marshaler.PtrToStringUtf8(DebugApi.GetExecutionTraceWrapper(lineCount)); }

		[DllImport(DllPath, EntryPoint = "GetState")] private static extern void GetStateWrapper(ref DebugState state);
		public static DebugState GetState()
		{
			DebugState state = new DebugState();
			DebugApi.GetStateWrapper(ref state);
			return state;
		}

		[DllImport(DllPath)] public static extern Int32 EvaluateExpression([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string expression, CpuType cpuType, out EvalResultType resultType, [MarshalAs(UnmanagedType.I1)]bool useCache);

		[DllImport(DllPath)] public static extern Int32 GetMemorySize(SnesMemoryType type);
		[DllImport(DllPath)] public static extern Byte GetMemoryValue(SnesMemoryType type, UInt32 address);
		[DllImport(DllPath)] public static extern void SetMemoryValue(SnesMemoryType type, UInt32 address, byte value);
		[DllImport(DllPath)] public static extern void SetMemoryValues(SnesMemoryType type, UInt32 address, [In] byte[] data, Int32 length);
		[DllImport(DllPath)] public static extern void SetMemoryState(SnesMemoryType type, [In] byte[] buffer, Int32 length);

		[DllImport(DllPath)] public static extern AddressInfo GetAbsoluteAddress(AddressInfo relAddress);
		[DllImport(DllPath)] public static extern AddressInfo GetRelativeAddress(AddressInfo absAddress);

		[DllImport(DllPath)] public static extern void SetLabel(uint address, SnesMemoryType memType, string label, string comment);
		[DllImport(DllPath)] public static extern void ClearLabels();

		[DllImport(DllPath)] public static extern void SetBreakpoints([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]InteropBreakpoint[] breakpoints, UInt32 length);

		[DllImport(DllPath, EntryPoint = "GetMemoryState")] private static extern void GetMemoryStateWrapper(SnesMemoryType type, [In, Out] byte[] buffer);
		public static byte[] GetMemoryState(SnesMemoryType type)
		{
			byte[] buffer = new byte[DebugApi.GetMemorySize(type)];
			DebugApi.GetMemoryStateWrapper(type, buffer);
			return buffer;
		}

		[DllImport(DllPath)] public static extern void GetTilemap(GetTilemapOptions options, byte[] vram, byte[] cgram, [In, Out] byte[] buffer);
		[DllImport(DllPath)] public static extern void GetTileView(GetTileViewOptions options, byte[] source, int srcSize, byte[] cgram, [In, Out] byte[] buffer);
		[DllImport(DllPath)] public static extern void GetSpritePreview(GetSpritePreviewOptions options, PpuState state, byte[] vram, byte[] oamRam, byte[] cgram, [In, Out] byte[] buffer);

		[DllImport(DllPath)] public static extern void SetViewerUpdateTiming(Int32 viewerId, Int32 scanline, Int32 cycle);

		[DllImport(DllPath)] private static extern UInt32 GetDebugEventCount([MarshalAs(UnmanagedType.I1)]bool getPreviousFrameData);
		[DllImport(DllPath, EntryPoint = "GetDebugEvents")] private static extern void GetDebugEventsWrapper([In, Out]DebugEventInfo[] eventArray, ref UInt32 maxEventCount, [MarshalAs(UnmanagedType.I1)]bool getPreviousFrameData);
		public static DebugEventInfo[] GetDebugEvents(bool getPreviousFrameData)
		{
			UInt32 maxEventCount = GetDebugEventCount(getPreviousFrameData);
			DebugEventInfo[] debugEvents = new DebugEventInfo[maxEventCount];

			DebugApi.GetDebugEventsWrapper(debugEvents, ref maxEventCount, getPreviousFrameData);
			if(maxEventCount < debugEvents.Length) {
				//Remove the excess from the array if needed
				Array.Resize(ref debugEvents, (int)maxEventCount);
			}

			return debugEvents;
		}

		[DllImport(DllPath)] public static extern DebugEventInfo GetEventViewerEvent(UInt16 scanline, UInt16 cycle, EventViewerDisplayOptions options);
		[DllImport(DllPath)] public static extern void TakeEventSnapshot(EventViewerDisplayOptions options);		

		[DllImport(DllPath, EntryPoint = "GetEventViewerOutput")] private static extern void GetEventViewerOutputWrapper([In, Out]byte[] buffer, EventViewerDisplayOptions options);
		public static byte[] GetEventViewerOutput(EventViewerDisplayOptions options)
		{
			byte[] buffer = new byte[340*2 * 262*2 * 4];
			DebugApi.GetEventViewerOutputWrapper(buffer, options);
			return buffer;
		}

		[DllImport(DllPath, EntryPoint = "GetCallstack")] private static extern void GetCallstackWrapper(CpuType type, [In, Out]StackFrameInfo[] callstackArray, ref UInt32 callstackSize);
		public static StackFrameInfo[] GetCallstack(CpuType type)
		{
			StackFrameInfo[] callstack = new StackFrameInfo[512];
			UInt32 callstackSize = 0;

			DebugApi.GetCallstackWrapper(type, callstack, ref callstackSize);
			Array.Resize(ref callstack, (int)callstackSize);

			return callstack;
		}

		[DllImport(DllPath, EntryPoint = "GetMemoryAccessStamps")] private static extern void GetMemoryAccessStampsWrapper(UInt32 offset, UInt32 length, SnesMemoryType type, MemoryOperationType operationType, [In,Out]UInt64[] stamps);
		public static UInt64[] GetMemoryAccessStamps(UInt32 offset, UInt32 length, SnesMemoryType type, MemoryOperationType operationType)
		{
			UInt64[] stamps = new UInt64[length];
			DebugApi.GetMemoryAccessStampsWrapper(offset, length, type, operationType, stamps);
			return stamps;
		}

		[DllImport(DllPath, EntryPoint = "GetMemoryAccessCounts")] private static extern void GetMemoryAccessCountsWrapper(UInt32 offset, UInt32 length, SnesMemoryType type, MemoryOperationType operationType, [In,Out]UInt32[] counts);
		public static UInt32[] GetMemoryAccessCounts(UInt32 offset, UInt32 length, SnesMemoryType type, MemoryOperationType operationType)
		{
			UInt32[] counts = new UInt32[length];
			DebugApi.GetMemoryAccessCountsWrapper(offset, length, type, operationType, counts);
			return counts;
		}

		[DllImport(DllPath, EntryPoint = "GetCdlData")] private static extern void GetCdlDataWrapper(UInt32 offset, UInt32 length, SnesMemoryType memType, [In,Out] byte[] cdlData);
		public static byte[] GetCdlData(UInt32 offset, UInt32 length, SnesMemoryType memType)
		{
			byte[] cdlData = new byte[length];
			DebugApi.GetCdlDataWrapper(offset, length, memType, cdlData);
			return cdlData;
		}

		[DllImport(DllPath)] public static extern void SetCdlData([In]byte[] cdlData, Int32 length);
	}

	public enum SnesMemoryType
	{
		CpuMemory,
		SpcMemory,
		PrgRom,
		WorkRam,
		SaveRam,
		VideoRam,
		SpriteRam,
		CGRam,
		SpcRam,
		SpcRom,
		Register
	}

	public static class SnesMemoryTypeExtensions
	{
		public static CpuType ToCpuType(this SnesMemoryType memType)
		{
			switch(memType) {
				case SnesMemoryType.SpcMemory:
				case SnesMemoryType.SpcRam:
				case SnesMemoryType.SpcRom:
					return CpuType.Spc;

				default:
					return CpuType.Cpu;
			}
		}

		public static bool SupportsLabels(this SnesMemoryType memType)
		{
			switch(memType) {
				case SnesMemoryType.PrgRom:
				case SnesMemoryType.WorkRam:
				case SnesMemoryType.SaveRam:
				case SnesMemoryType.Register:
				case SnesMemoryType.SpcRam:
				case SnesMemoryType.SpcRom:
					return true;
			}

			return false;
		}

		public static bool SupportsWatch(this SnesMemoryType memType)
		{
			switch(memType) {
				case SnesMemoryType.CpuMemory:
				case SnesMemoryType.SpcMemory:
					return true;
			}

			return false;
		}
	}

	public struct AddressInfo
	{
		public Int32 Address;
		public SnesMemoryType Type;
	}

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
		[MarshalAs(UnmanagedType.I1)] public bool OverscanMode;

		public byte BgMode;
		[MarshalAs(UnmanagedType.I1)] public bool DirectColorMode;

		public Mode7Config Mode7;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
		public LayerConfig[] Layers;

		public byte OamMode;
		public UInt16 OamBaseAddress;
		public UInt16 OamAddressOffset;
		[MarshalAs(UnmanagedType.I1)] public bool EnableOamPriority;
		[MarshalAs(UnmanagedType.I1)] public bool ObjInterlace;
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
		[MarshalAs(UnmanagedType.I1)] public bool ExtBgEnabled;
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

	public struct DebugState
	{
		public UInt64 MasterClock;
		public CpuState Cpu;
		public PpuState Ppu;
		public SpcState Spc;
	}

	public enum MemoryOperationType
	{
		Read = 0,
		Write = 1,
		ExecOpCode = 2,
		ExecOperand = 3,
		DmaRead = 4,
		DmaWrite = 5,
		DummyRead = 6
	}

	public struct MemoryOperationInfo
	{
		public UInt32 Address;
		public Int32 Value;
		public MemoryOperationType Type;
	}

	public enum DebugEventType
	{
		Register,
		Nmi,
		Irq,
		Breakpoint
	}

	public struct DmaChannelConfig
	{
		public byte InvertDirection;
		public byte Decrement;
		public byte FixedTransfer;
		public byte HdmaIndirectAddressing;
		public byte TransferMode;

		public UInt16 SrcAddress;
		public byte SrcBank;

		public UInt16 TransferSize;
		public byte DestAddress;

		public UInt16 HdmaTableAddress;
		public byte HdmaBank;
		public byte HdmaLineCounterAndRepeat;
		public byte DoTransfer;
		public byte HdmaFinished;

		public byte InterruptedByHdma;
		public byte UnusedFlag;
	}

	public struct DebugEventInfo
	{
		public MemoryOperationInfo Operation;
		public DebugEventType Type;
		public UInt32 ProgramCounter;
		public UInt16 Scanline;
		public UInt16 Cycle;
		public UInt16 BreakpointId;
		public byte DmaChannel;
		public DmaChannelConfig DmaChannelInfo;
	};

	public struct EventViewerDisplayOptions
	{
		[MarshalAs(UnmanagedType.I1)] public bool ShowPpuRegisterWrites;
		[MarshalAs(UnmanagedType.I1)] public bool ShowPpuRegisterReads;
		[MarshalAs(UnmanagedType.I1)] public bool ShowCpuRegisterWrites;
		[MarshalAs(UnmanagedType.I1)] public bool ShowCpuRegisterReads;

		[MarshalAs(UnmanagedType.I1)] public bool ShowApuRegisterWrites;
		[MarshalAs(UnmanagedType.I1)] public bool ShowApuRegisterReads;
		[MarshalAs(UnmanagedType.I1)] public bool ShowWorkRamRegisterWrites;
		[MarshalAs(UnmanagedType.I1)] public bool ShowWorkRamRegisterReads;

		[MarshalAs(UnmanagedType.I1)] public bool ShowNmi;
		[MarshalAs(UnmanagedType.I1)] public bool ShowIrq;

		[MarshalAs(UnmanagedType.I1)] public bool ShowMarkedBreakpoints;
		[MarshalAs(UnmanagedType.I1)] public bool ShowPreviousFrameEvents;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 8, ArraySubType = UnmanagedType.I1)]
		public bool[] ShowDmaChannels;

		public UInt32 IrqColor;
		public UInt32 NmiColor;
		public UInt32 BreakpointColor;
		public UInt32 PpuRegisterReadColor;
		public UInt32 PpuRegisterWriteColor;
		public UInt32 ApuRegisterReadColor;
		public UInt32 ApuRegisterWriteColor;
		public UInt32 CpuRegisterReadColor;
		public UInt32 CpuRegisterWriteColor;
		public UInt32 WorkRamRegisterReadColor;
		public UInt32 WorkRamRegisterWriteColor;
	}

	public struct GetTilemapOptions
	{
		public byte BgMode;
		public byte Layer;

		public byte Bpp;
		public Int32 TilemapAddr;
		public Int32 ChrAddr;

		[MarshalAs(UnmanagedType.I1)] public bool ShowTileGrid;
		[MarshalAs(UnmanagedType.I1)] public bool ShowScrollOverlay;
	}

	public struct GetTileViewOptions
	{
		public TileFormat Format;
		public Int32 Width;
		public Int32 Palette;

		[MarshalAs(UnmanagedType.I1)] public bool ShowTileGrid;
	}

	public struct GetSpritePreviewOptions
	{
		public Int32 SelectedSprite;
	}

	public enum TileFormat
	{
		Bpp2,
		Bpp4,
		Bpp8,
		DirectColor,
		Mode7,
		Mode7DirectColor,
	}

	[Serializable]
	public struct InteropTraceLoggerOptions
	{
		[MarshalAs(UnmanagedType.I1)] public bool LogCpu;
		[MarshalAs(UnmanagedType.I1)] public bool LogSpc;

		[MarshalAs(UnmanagedType.I1)] public bool ShowExtraInfo;
		[MarshalAs(UnmanagedType.I1)] public bool IndentCode;
		[MarshalAs(UnmanagedType.I1)] public bool UseLabels;
		[MarshalAs(UnmanagedType.I1)] public bool UseWindowsEol;
		[MarshalAs(UnmanagedType.I1)] public bool ExtendZeroPage;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 1000)]
		public byte[] Condition;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 1000)]
		public byte[] Format;
	}

	public enum EvalResultType
	{
		Numeric = 0,
		Boolean = 1,
		Invalid = 2,
		DivideBy0 = 3,
		OutOfScope = 4
	}
	
	public struct StackFrameInfo
	{
		public UInt32 Source;
		public UInt32 Target;
		public UInt32 Return;
		public StackFrameFlags Flags;
	};

	public enum StackFrameFlags
	{
		None = 0,
		Nmi = 1,
		Irq = 2
	}
	
	public enum CpuType
	{
		Cpu,
		Spc,
	}

	public enum StepType
	{
		CpuStep,
		CpuStepOut,
		CpuStepOver,
		SpcStep,
		SpcStepOut,
		SpcStepOver,
		PpuStep,
		SpecificScanline,
	}

	public enum BreakSource
	{
		Unspecified = -1,
		Breakpoint = 0,
		CpuStep = 1,
		PpuStep = 2,
		BreakOnBrk = 3,
		BreakOnCop = 4,
		BreakOnWdm = 5,
		BreakOnStp = 6
	}

	public struct BreakEvent
	{
		public BreakSource Source;
		public MemoryOperationInfo Operation;
		public Int32 BreakpointId;
	}

	public enum CdlFlags : byte
	{
		None = 0x00,
		Code = 0x01,
		Data = 0x02,
		JumpTarget = 0x04,
		SubEntryPoint = 0x08,

		IndexMode8 = 0x10,
		MemoryMode8 = 0x20,
	}
}
