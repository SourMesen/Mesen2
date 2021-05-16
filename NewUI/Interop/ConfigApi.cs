using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Mesen.Config;
using Mesen.Config.Shortcuts;
using Mesen.Utilities;

namespace Mesen.Interop
{
	public class ConfigApi
	{
		private const string DllPath = "MesenSCore.dll";

		[DllImport(DllPath)] public static extern void SetVideoConfig(InteropVideoConfig config);
		[DllImport(DllPath)] public static extern void SetAudioConfig(InteropAudioConfig config);
		[DllImport(DllPath)] public static extern void SetInputConfig(InteropInputConfig config);
		[DllImport(DllPath)] public static extern void SetEmulationConfig(InteropEmulationConfig config);
		[DllImport(DllPath)] public static extern void SetGameboyConfig(InteropGameboyConfig config);
		[DllImport(DllPath)] public static extern void SetNesConfig(InteropNesConfig config);
		[DllImport(DllPath)] public static extern void SetSnesConfig(InteropSnesConfig config);

		[DllImport(DllPath)] public static extern void SetPreferences(InteropPreferencesConfig config);
		[DllImport(DllPath)] public static extern void SetAudioPlayerConfig(InteropAudioPlayerConfig config);
		[DllImport(DllPath)] public static extern void SetShortcutKeys([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]InteropShortcutKeyInfo[] shortcuts, UInt32 count);

		[DllImport(DllPath)] public static extern void SetEmulationFlag(EmulationFlags flag, bool enabled);
		[DllImport(DllPath)] public static extern void SetDebuggerFlag(DebuggerFlags flag, bool enabled);

		[DllImport(DllPath)] public static extern ControllerType GetControllerType(int player);

		[DllImport(DllPath, EntryPoint = "GetAudioDevices")] private static extern IntPtr GetAudioDevicesWrapper();
		public static List<string> GetAudioDevices()
		{
			return new List<string>(Utf8Utilities.PtrToStringUtf8(ConfigApi.GetAudioDevicesWrapper()).Split(new string[1] { "||" }, StringSplitOptions.RemoveEmptyEntries));
		}
	}

	public enum EmulationFlags : UInt32
	{
		Turbo = 1,
		Rewind = 2,
		MaximumSpeed = 4,
		InBackground = 8,
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

		UseAltSpcOpNames = 0x1000,
		UseLowerCaseDisassembly = 0x2000,
		
		AutoResetCdl = 0x4000,

		GbBreakOnInvalidOamAccess = 0x10000,
		GbBreakOnInvalidVramAccess = 0x20000,
		GbBreakOnDisableLcdOutsideVblank = 0x40000,
		GbBreakOnInvalidOpCode = 0x80000,
		GbBreakOnNopLoad = 0x100000,
		GbBreakOnOamCorruption = 0x200000,

		NesDebuggerEnabled = 0x01000000,
		GbDebuggerEnabled = 0x02000000,
		Cx4DebuggerEnabled = 0x04000000,
		NecDspDebuggerEnabled = 0x08000000,
		GsuDebuggerEnabled = 0x10000000,
		Sa1DebuggerEnabled = 0x20000000,
		SpcDebuggerEnabled = 0x40000000,
		CpuDebuggerEnabled = 0x80000000
	}

	public struct InteropShortcutKeyInfo
	{
		public EmulatorShortcut Shortcut;
		public InteropKeyCombination KeyCombination;

		public InteropShortcutKeyInfo(EmulatorShortcut key, InteropKeyCombination keyCombination)
		{
			Shortcut = key;
			KeyCombination = keyCombination;
		}
	}

	public struct InteropKeyCombination
	{
		public UInt32 Key1;
		public UInt32 Key2;
		public UInt32 Key3;
	}

}
