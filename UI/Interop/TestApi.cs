using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Interop
{
	class TestApi
	{
		private const string DllPath = EmuApi.DllName;

		[DllImport(DllPath)] public static extern RomTestResult RunRecordedTest([MarshalAs(UnmanagedType.LPUTF8Str)]string filename, [MarshalAs(UnmanagedType.I1)]bool inBackground);
		[DllImport(DllPath)] public static extern UInt64 RunTest([MarshalAs(UnmanagedType.LPUTF8Str)]string filename, int address, MemoryType memType);
		[DllImport(DllPath)] public static extern void RomTestRecord([MarshalAs(UnmanagedType.LPUTF8Str)]string filename, [MarshalAs(UnmanagedType.I1)]bool reset);
		[DllImport(DllPath)] public static extern void RomTestStop();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool RomTestRecording();
	}

	public struct RomTestResult
	{
		public RomTestState State;
		public Int32 ErrorCode;
	}

	public enum RomTestState
	{
		Failed,
		Passed,
		PassedWithWarnings
	}
}
