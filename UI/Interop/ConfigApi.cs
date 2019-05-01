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
using Mesen.GUI.Config.Shortcuts;
using Mesen.GUI.Forms;

namespace Mesen.GUI
{
	public class ConfigApi
	{
		private const string DllPath = "MesenSCore.dll";

		[DllImport(DllPath)] public static extern void SetVideoConfig(VideoConfig config);
		[DllImport(DllPath)] public static extern void SetAudioConfig(AudioConfig config);
		[DllImport(DllPath)] public static extern void SetInputConfig(InputConfig config);
		[DllImport(DllPath)] public static extern void SetEmulationConfig(EmulationConfig config);

		[DllImport(DllPath)] public static extern void SetPreferences(InteropPreferencesConfig config);
		[DllImport(DllPath)] public static extern void SetShortcutKeys([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]ShortcutKeyInfo[] shortcuts, UInt32 count);

		[DllImport(DllPath)] public static extern void SetDebuggerFlag(DebuggerFlags flag, bool enabled);

		[DllImport(DllPath, EntryPoint = "GetAudioDevices")] private static extern IntPtr GetAudioDevicesWrapper();
		public static List<string> GetAudioDevices()
		{
			return new List<string>(Utf8Marshaler.PtrToStringUtf8(ConfigApi.GetAudioDevicesWrapper()).Split(new string[1] { "||" }, StringSplitOptions.RemoveEmptyEntries));
		}
	}
	
	public enum DebuggerFlags : UInt32
	{
		BreakOnBrk = 0x01,
		BreakOnCop = 0x02,
		BreakOnWdm = 0x04,
		BreakOnStp = 0x08,
		BreakOnUninitRead = 0x10,

		ShowVerifiedData = 0x100,
		DisassembleVerifiedData = 0x200,

		ShowUnidentifiedData = 0x400,
		DisassembleUnidentifiedData = 0x800,

		SpcDebuggerEnabled = 0x40000000,
		CpuDebuggerEnabled = 0x80000000
	}
}
