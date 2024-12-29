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
		private const string DllPath = EmuApi.DllName;

		[DllImport(DllPath)] public static extern void SetVideoConfig(InteropVideoConfig config);
		[DllImport(DllPath)] public static extern void SetAudioConfig(InteropAudioConfig config);
		[DllImport(DllPath)] public static extern void SetInputConfig(InteropInputConfig config);
		[DllImport(DllPath)] public static extern void SetEmulationConfig(InteropEmulationConfig config);
		
		[DllImport(DllPath)] public static extern void SetGameboyConfig(InteropGameboyConfig config);
		[DllImport(DllPath)] public static extern void SetGbaConfig(InteropGbaConfig config);
		[DllImport(DllPath)] public static extern void SetPcEngineConfig(InteropPcEngineConfig config);
		[DllImport(DllPath)] public static extern void SetNesConfig(InteropNesConfig config);
		[DllImport(DllPath)] public static extern void SetSnesConfig(InteropSnesConfig config);
		[DllImport(DllPath)] public static extern void SetSmsConfig(InteropSmsConfig config);
		[DllImport(DllPath)] public static extern void SetCvConfig(InteropCvConfig config);
		[DllImport(DllPath)] public static extern void SetWsConfig(InteropWsConfig config);

		[DllImport(DllPath)] public static extern void SetGameConfig(InteropGameConfig config);

		[DllImport(DllPath)] public static extern void SetPreferences(InteropPreferencesConfig config);
		[DllImport(DllPath)] public static extern void SetAudioPlayerConfig(InteropAudioPlayerConfig config);
		[DllImport(DllPath)] public static extern void SetShortcutKeys([MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)]InteropShortcutKeyInfo[] shortcuts, UInt32 count);

		[DllImport(DllPath)] public static extern void SetDebugConfig(InteropDebugConfig config);

		[DllImport(DllPath)] public static extern void SetEmulationFlag(EmulationFlags flag, bool enabled);
		[DllImport(DllPath)] public static extern void SetDebuggerFlag(DebuggerFlags flag, bool enabled);

		[DllImport(DllPath)] public static extern InteropNesConfig GetNesConfig();

		[DllImport(DllPath, EntryPoint = "GetAudioDevices")] private static extern void GetAudioDevicesWrapper(IntPtr outDeviceList, Int32 maxSize);
		public unsafe static List<string> GetAudioDevices()
		{
			return Utf8Utilities.CallStringApi(GetAudioDevicesWrapper).Split(new String[1] { "||" }, StringSplitOptions.RemoveEmptyEntries).ToList();
		}
	}

	public enum EmulationFlags : UInt32
	{
		Turbo = 0x01,
		Rewind = 0x02,
		MaximumSpeed = 0x04,
		InBackground = 0x08,
		ConsoleMode = 0x10,
	}

	public enum DebuggerFlags : UInt32
	{
		SnesDebuggerEnabled = (1 << 0),
		SpcDebuggerEnabled = (1 << 1),
		Sa1DebuggerEnabled = (1 << 2),
		GsuDebuggerEnabled = (1 << 3),
		NecDspDebuggerEnabled = (1 << 4),
		Cx4DebuggerEnabled = (1 << 5),
		St018DebuggerEnabled = (1 << 6),
		GbDebuggerEnabled = (1 << 7),
		NesDebuggerEnabled = (1 << 8),
		PceDebuggerEnabled = (1 << 9),
		SmsDebuggerEnabled = (1 << 10),
		GbaDebuggerEnabled = (1 << 11),
		WsDebuggerEnabled = (1 << 12),
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
