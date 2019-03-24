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
		[DllImport(DllPath)] public static extern void Step(Int32 instructionCount);

		[DllImport(DllPath)] public static extern void StartTraceLogger([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string filename);
		[DllImport(DllPath)] public static extern void StopTraceLogger();
		[DllImport(DllPath)] public static extern void SetTraceOptions(InteropTraceLoggerOptions options);

		[DllImport(DllPath, EntryPoint = "GetDisassemblyLineData")] private static extern void GetDisassemblyLineDataWrapper(UInt32 lineIndex, ref InteropCodeLineData lineData);
		public static CodeLineData GetDisassemblyLineData(UInt32 lineIndex)
		{
			InteropCodeLineData data = new InteropCodeLineData();
			data.Comment = new byte[1000];
			data.Text = new byte[1000];
			data.ByteCode = new byte[4];

			DebugApi.GetDisassemblyLineDataWrapper(lineIndex, ref data);
			return new CodeLineData(data);
		}

		[DllImport(DllPath)] public static extern UInt32 GetDisassemblyLineCount();
		[DllImport(DllPath)] public static extern UInt32 GetDisassemblyLineIndex(UInt32 cpuAddress);
		[DllImport(DllPath)] public static extern int SearchDisassembly([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string searchString, int startPosition, int endPosition, [MarshalAs(UnmanagedType.I1)]bool searchBackwards);

		[DllImport(DllPath, EntryPoint = "GetExecutionTrace")] private static extern IntPtr GetExecutionTraceWrapper(UInt32 lineCount);
		public static string GetExecutionTrace(UInt32 lineCount) { return Utf8Marshaler.PtrToStringUtf8(DebugApi.GetExecutionTraceWrapper(lineCount)); }

		[DllImport(DllPath, EntryPoint = "GetState")] private static extern void GetStateWrapper(ref DebugState state);
		public static DebugState GetState()
		{
			DebugState state = new DebugState();
			DebugApi.GetStateWrapper(ref state);
			return state;
		}

		[DllImport(DllPath)] public static extern Int32 EvaluateExpression([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string expression, out EvalResultType resultType, [MarshalAs(UnmanagedType.I1)]bool useCache);

		[DllImport(DllPath)] public static extern Int32 GetMemorySize(SnesMemoryType type);
		[DllImport(DllPath)] public static extern Byte GetMemoryValue(SnesMemoryType type, UInt32 address);
		[DllImport(DllPath)] public static extern void SetMemoryValue(SnesMemoryType type, UInt32 address, byte value);
		[DllImport(DllPath)] public static extern void SetMemoryValues(SnesMemoryType type, UInt32 address, [In] byte[] data, Int32 length);
		[DllImport(DllPath)] public static extern void SetMemoryState(SnesMemoryType type, [In] byte[] buffer, Int32 length);

		[DllImport(DllPath)] public static extern void SetBreakpoints([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]InteropBreakpoint[] breakpoints, UInt32 length);

		[DllImport(DllPath, EntryPoint = "GetMemoryState")] private static extern void GetMemoryStateWrapper(SnesMemoryType type, [In, Out] byte[] buffer);
		public static byte[] GetMemoryState(SnesMemoryType type)
		{
			byte[] buffer = new byte[DebugApi.GetMemorySize(type)];
			DebugApi.GetMemoryStateWrapper(type, buffer);
			return buffer;
		}

		[DllImport(DllPath)] public static extern void GetTilemap(GetTilemapOptions options, [In, Out] byte[] buffer);
		[DllImport(DllPath)] public static extern void GetTileView(GetTileViewOptions options, [In, Out] byte[] buffer);

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

		[DllImport(DllPath, EntryPoint = "GetCallstack")] private static extern void DebugGetCallstackWrapper([In, Out]StackFrameInfo[] callstackArray, ref UInt32 callstackSize);
		public static StackFrameInfo[] GetCallstack()
		{
			StackFrameInfo[] callstack = new StackFrameInfo[512];
			UInt32 callstackSize = 0;

			DebugApi.DebugGetCallstackWrapper(callstack, ref callstackSize);
			Array.Resize(ref callstack, (int)callstackSize);

			return callstack;
		}
	}

	public enum SnesMemoryType
	{
		CpuMemory,
		PrgRom,
		WorkRam,
		SaveRam,
		VideoRam,
		SpriteRam,
		CGRam,
		Register,
		SpcRam
	}

	public class AddressInfo
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
		public UInt32 FrameCount;
		[MarshalAs(UnmanagedType.I1)] public bool OverscanMode;

		public byte BgMode;
		[MarshalAs(UnmanagedType.I1)] public bool DirectColorMode;

		public Mode7Config Mode7;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
		public LayerConfig[] Layers;
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

	public struct DebugState
	{
		public CpuState Cpu;
		public PpuState Ppu;
	}

	public enum MemoryOperationType
	{
		Read = 0,
		Write = 1,
		ExecOpCode = 2,
		ExecOperand = 3,
		DmaRead = 4,
		DmaWrite = 5
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

	public struct DebugEventInfo
	{
		//public DmaChannelConfig DmaChannelInfo;
		public MemoryOperationInfo Operation;
		public DebugEventType Type;
		public UInt32 ProgramCounter;
		public UInt16 Scanline;
		public UInt16 Cycle;
		public UInt16 BreakpointId;
		//public byte DmaChannel;
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
		public SnesMemoryType MemoryType;
		public Int32 AddressOffset;

		[MarshalAs(UnmanagedType.I1)] public bool ShowTileGrid;
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
}
