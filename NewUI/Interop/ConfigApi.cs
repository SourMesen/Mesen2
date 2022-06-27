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
		[DllImport(DllPath)] public static extern void SetPcEngineConfig(InteropPcEngineConfig config);
		[DllImport(DllPath)] public static extern void SetNesConfig(InteropNesConfig config);
		[DllImport(DllPath)] public static extern void SetSnesConfig(InteropSnesConfig config);

		[DllImport(DllPath)] public static extern void SetPreferences(InteropPreferencesConfig config);
		[DllImport(DllPath)] public static extern void SetAudioPlayerConfig(InteropAudioPlayerConfig config);
		[DllImport(DllPath)] public static extern void SetShortcutKeys([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]InteropShortcutKeyInfo[] shortcuts, UInt32 count);

		[DllImport(DllPath)] public static extern void SetEmulationFlag(EmulationFlags flag, bool enabled);
		[DllImport(DllPath)] public static extern void SetDebuggerFlag(DebuggerFlags flag, bool enabled);

		[DllImport(DllPath)] public static extern ControllerType GetControllerType(int player);

		[DllImport(DllPath, EntryPoint = "GetAudioDevices")] private static extern void GetAudioDevicesWrapper(IntPtr outDeviceList, Int32 maxSize);
		public unsafe static List<string> GetAudioDevices()
		{
			return Utf8Utilities.CallStringApi(GetAudioDevicesWrapper).Split(new String[1] { "||" }, StringSplitOptions.RemoveEmptyEntries).ToList();
		}
	}

	public enum EmulationFlags : UInt32
	{
		Turbo = 1,
		Rewind = 2,
		MaximumSpeed = 4,
		InBackground = 8,
	}

	public enum DebuggerFlags : UInt64
	{
		BreakOnBrk = (1 << 0),
		BreakOnCop = (1 << 1),
		BreakOnWdm = (1 << 2),
		BreakOnStp = (1 << 3),
		BreakOnUninitRead = (1 << 4),

		ShowJumpLabels = (1 << 5),
		DrawPartialFrame = (1 << 6),

		ShowVerifiedData = (1 << 8),
		DisassembleVerifiedData = (1 << 9),

		ShowUnidentifiedData = (1 << 10),
		DisassembleUnidentifiedData = (1 << 11),

		UseAltSpcOpNames = (1 << 12),
		UseLowerCaseDisassembly = (1 << 13),
		
		AutoResetCdl = (1 << 14),

		GbBreakOnInvalidOamAccess = (1 << 16),
		GbBreakOnInvalidVramAccess = (1 << 17),
		GbBreakOnDisableLcdOutsideVblank = (1 << 18),
		GbBreakOnInvalidOpCode = (1 << 19),
		GbBreakOnNopLoad = (1 << 20),
		GbBreakOnOamCorruption = (1 << 21),

		PceDebuggerEnabled = (1 << 23),
		NesDebuggerEnabled = (1 << 24),
		GbDebuggerEnabled = (1 << 25),
		Cx4DebuggerEnabled = (1 << 26),
		NecDspDebuggerEnabled = (1 << 27),
		GsuDebuggerEnabled = (1 << 28),
		Sa1DebuggerEnabled = (1 << 29),
		SpcDebuggerEnabled = (1 << 30),
		SnesDebuggerEnabled = (1LU << 31),

		NesBreakOnBrk = (1LU << 32),
		NesBreakOnUnofficialOpCode = (1LU << 33),
		NesBreakOnCpuCrash = (1LU << 34),
		NesBreakOnBusConflict = (1LU << 35),
		NesBreakOnDecayedOamRead = (1LU << 36),
		NesBreakOnPpu2006ScrollGlitch = (1LU << 37),
		
		ScriptAllowIoOsAccess = (1LU << 40),
		ScriptAllowNetworkAccess = (1LU << 41),

		UsePredictiveBreakpoints = (1LU << 42),
		SingleBreakpointPerInstruction = (1LU << 43),

		PceBreakOnBrk = (1LU << 44),
		PceBreakOnUnofficialOpCode = (1LU << 45),
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
