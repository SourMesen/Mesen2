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

		[DllImport(DllPath, EntryPoint = "GetExecutionTrace")] private static extern IntPtr GetExecutionTraceWrapper(UInt32 lineCount);
		public static string GetExecutionTrace(UInt32 lineCount) { return Utf8Marshaler.PtrToStringUtf8(DebugApi.GetExecutionTraceWrapper(lineCount)); }

		[DllImport(DllPath, EntryPoint = "GetState")] private static extern void GetStateWrapper(ref DebugState state);
		public static DebugState GetState()
		{
			DebugState state = new DebugState();
			DebugApi.GetStateWrapper(ref state);
			return state;
		}
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
