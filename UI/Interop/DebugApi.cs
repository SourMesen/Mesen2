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

		[DllImport(DllPath)] public static extern Int32 GetMemorySize(SnesMemoryType type);
		[DllImport(DllPath)] public static extern Byte GetMemoryValue(SnesMemoryType type, UInt32 address);
		[DllImport(DllPath)] public static extern void SetMemoryValue(SnesMemoryType type, UInt32 address, byte value);
		[DllImport(DllPath)] public static extern void SetMemoryValues(SnesMemoryType type, UInt32 address, [In] byte[] data, Int32 length);
		[DllImport(DllPath)] public static extern void SetMemoryState(SnesMemoryType type, [In] byte[] buffer, Int32 length);

		[DllImport(DllPath, EntryPoint = "GetMemoryState")] private static extern void GetMemoryStateWrapper(SnesMemoryType type, [In, Out] byte[] buffer);
		public static byte[] GetMemoryState(SnesMemoryType type)
		{
			byte[] buffer = new byte[DebugApi.GetMemorySize(type)];
			DebugApi.GetMemoryStateWrapper(type, buffer);
			return buffer;
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
	}

	public class AddressInfo
	{
		public Int32 Address;
		public SnesMemoryType Type;
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
		public byte PS;
		[MarshalAs(UnmanagedType.I1)] public bool EmulationMode;
	};

	public struct PpuState
	{
		public UInt16 Cycle;
		public UInt16 Scanline;
		public UInt32 FrameCount;
	};

	public struct DebugState
	{
		public CpuState Cpu;
		public PpuState Ppu;
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
}
