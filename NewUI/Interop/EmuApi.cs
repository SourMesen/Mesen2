using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Mesen.Localization;
using Mesen.Config.Shortcuts;
using Mesen.Utilities;
using Avalonia.Media.Imaging;

namespace Mesen.Interop
{
	public class EmuApi
	{
		private const string DllPath = "MesenSCore.dll";
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool TestDll();
		[DllImport(DllPath)] public static extern void InitDll();

		[DllImport(DllPath, EntryPoint = "GetMesenVersion")] private static extern UInt32 GetMesenVersionWrapper();
		public static Version GetMesenVersion()
		{
			UInt32 version = GetMesenVersionWrapper();
			UInt32 revision = version & 0xFF;
			UInt32 minor = (version >> 8) & 0xFF;
			UInt32 major = (version >> 16) & 0xFFFF;
			return new Version((int)major, (int)minor, (int)revision);
		}

		[DllImport(DllPath)] public static extern IntPtr RegisterNotificationCallback(NotificationListener.NotificationCallback callback);
		[DllImport(DllPath)] public static extern void UnregisterNotificationCallback(IntPtr notificationListener);

		[DllImport(DllPath)] public static extern void InitializeEmu([MarshalAs(UnmanagedType.LPUTF8Str)]string homeFolder, IntPtr windowHandle, IntPtr dxViewerHandle, [MarshalAs(UnmanagedType.I1)]bool noAudio, [MarshalAs(UnmanagedType.I1)]bool noVideo, [MarshalAs(UnmanagedType.I1)]bool noInput);

		[DllImport(DllPath)] public static extern void Release();

		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool IsRunning();
		[DllImport(DllPath)] public static extern void Stop();

		[DllImport(DllPath)] public static extern void Reset();
		[DllImport(DllPath)] public static extern void PowerCycle();
		[DllImport(DllPath)] public static extern void ReloadRom();

		[DllImport(DllPath)] public static extern void Pause();
		[DllImport(DllPath)] public static extern void Resume();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool IsPaused();

		[DllImport(DllPath)] public static extern void TakeScreenshot();

		[DllImport(DllPath)] public static extern void ProcessAudioPlayerAction(AudioPlayerActionParams p);

		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool LoadRom(
			[MarshalAs(UnmanagedType.LPUTF8Str)]string filepath,
			[MarshalAs(UnmanagedType.LPUTF8Str)]string? patchFile = null
		);

		[DllImport(DllPath, EntryPoint = "GetRomInfo")] private static extern void GetRomInfoWrapper(out InteropRomInfo romInfo);
		public static RomInfo GetRomInfo()
		{
			InteropRomInfo info;
			EmuApi.GetRomInfoWrapper(out info);
			return new RomInfo(info);
		}

		[DllImport(DllPath)] public static extern void LoadRecentGame([MarshalAs(UnmanagedType.LPUTF8Str)]string filepath, [MarshalAs(UnmanagedType.I1)]bool resetGame);

		[DllImport(DllPath)] public static extern void AddKnownGameFolder([MarshalAs(UnmanagedType.LPUTF8Str)]string folder);

		[DllImport(DllPath)] public static extern void SetFolderOverrides(
			[MarshalAs(UnmanagedType.LPUTF8Str)]string saveDataFolder,
			[MarshalAs(UnmanagedType.LPUTF8Str)]string saveStateFolder,
			[MarshalAs(UnmanagedType.LPUTF8Str)]string screenshotFolder
		);

		[DllImport(DllPath)] public static extern void SetDisplayLanguage(Language lang);
		[DllImport(DllPath)] public static extern void SetFullscreenMode([MarshalAs(UnmanagedType.I1)]bool fullscreen, IntPtr windowHandle, UInt32 monitorWidth, UInt32 monitorHeight);

		[DllImport(DllPath)] public static extern TimingInfo GetTimingInfo();

		[DllImport(DllPath)] public static extern double GetAspectRatio();
		[DllImport(DllPath)] public static extern FrameInfo GetBaseScreenSize();
		[DllImport(DllPath)] public static extern void SetRendererSize(UInt32 width, UInt32 height);

		[DllImport(DllPath)] public static extern void ExecuteShortcut(ExecuteShortcutParams p);
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool IsShortcutAllowed(EmulatorShortcut shortcut, UInt32 shortcutParam = 0);

		[DllImport(DllPath, EntryPoint = "GetLog")] private static extern IntPtr GetLogWrapper();
		public static string GetLog() { return Utf8Utilities.PtrToStringUtf8(EmuApi.GetLogWrapper()).Replace("\n", Environment.NewLine); }
		[DllImport(DllPath)] public static extern void WriteLogEntry([MarshalAs(UnmanagedType.LPUTF8Str)]string message);
		[DllImport(DllPath)] public static extern void DisplayMessage([MarshalAs(UnmanagedType.LPUTF8Str)]string title, [MarshalAs(UnmanagedType.LPUTF8Str)]string message, [MarshalAs(UnmanagedType.LPUTF8Str)]string? param1 = null);

		[DllImport(DllPath)] public static extern IntPtr GetArchiveRomList([MarshalAs(UnmanagedType.LPUTF8Str)]string filename);

		[DllImport(DllPath)] public static extern void SaveState(UInt32 stateIndex);
		[DllImport(DllPath)] public static extern void LoadState(UInt32 stateIndex);
		[DllImport(DllPath)] public static extern void SaveStateFile([MarshalAs(UnmanagedType.LPUTF8Str)]string filepath);
		[DllImport(DllPath)] public static extern void LoadStateFile([MarshalAs(UnmanagedType.LPUTF8Str)]string filepath);

		[DllImport(DllPath, EntryPoint = "GetSaveStatePreview")] private static extern Int32 GetSaveStatePreviewWrapper([MarshalAs(UnmanagedType.LPUTF8Str)]string saveStatePath, [Out]byte[] imgData);
		public static Bitmap? GetSaveStatePreview(string saveStatePath)
		{
			if(File.Exists(saveStatePath)) {
				byte[] buffer = new byte[512*478*4];
				Int32 size = EmuApi.GetSaveStatePreviewWrapper(saveStatePath, buffer);
				if(size > 0) {
					Array.Resize(ref buffer, size);
					using(MemoryStream stream = new MemoryStream(buffer)) {
						return new Bitmap(stream);
					}
				}
			}
			return null;
		}

		[DllImport(DllPath)] public static extern void SetCheats([In]UInt32[] cheats, UInt32 cheatCount);
		[DllImport(DllPath)] public static extern void ClearCheats();

		[DllImport(DllPath)] public static extern void InputBarcode(UInt64 barcode, UInt32 digitCount);
		[DllImport(DllPath)] public static extern void ProcessTapeRecorderAction(TapeRecorderAction action, [MarshalAs(UnmanagedType.LPUTF8Str)] string filename = "");
	}

	public struct TimingInfo
	{
		public double Fps;
		public UInt64 MasterClock;
		public UInt32 MasterClockRate;
		public UInt32 FrameCount;

		public UInt32 ScanlineCount;
		public Int32 FirstScanline;
		public UInt32 CycleCount;
	}

	public struct FrameInfo
	{
		public UInt32 Width;
		public UInt32 Height;
	}

	public enum RomFormat
	{
		Unknown,

		Sfc,
		SfcWithCopierHeader,
		Spc,

		Gb,
		Gbs,

		iNes,
		Unif,
		Fds,
		VsSystem,
		VsDualSystem,
		Nsf,
		StudyBox
	}

	public enum ConsoleType
	{
		Snes = 0,
		Gameboy = 1,
		GameboyColor = 2,
		Nes = 3
	}

	public struct InteropRomInfo
	{
		public IntPtr RomPath;
		public IntPtr PatchPath;
		public RomFormat Format;
		public ConsoleType ConsoleType;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5)]
		public CpuType[] CpuTypes;
		public UInt32 CpuTypeCount;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 40)]
		public byte[] Sha1;
	}

	public class RomInfo
	{
		public string RomPath = "";
		public string PatchPath = "";
		public RomFormat Format = RomFormat.Unknown;
		public ConsoleType ConsoleType = ConsoleType.Snes;
		public string Sha1 = "";
		public HashSet<CpuType> CpuTypes = new HashSet<CpuType>();

		public RomInfo() { }

		public RomInfo(InteropRomInfo romInfo)
		{
			RomPath = (ResourcePath)Utf8Utilities.GetStringFromIntPtr(romInfo.RomPath);
			PatchPath = (ResourcePath)Utf8Utilities.GetStringFromIntPtr(romInfo.PatchPath);
			Format = romInfo.Format;
			ConsoleType = romInfo.ConsoleType;
			Sha1 = Encoding.UTF8.GetString(romInfo.Sha1);

			for(int i = 0; i < romInfo.CpuTypeCount; i++) {
				CpuTypes.Add(romInfo.CpuTypes[i]);
			}
		}

		public string GetRomName()
		{
			return Path.GetFileNameWithoutExtension(((ResourcePath)RomPath).FileName);
		}
	}

	public enum FirmwareType
	{
		CX4,
		DSP1,
		DSP1B,
		DSP2,
		DSP3,
		DSP4,
		ST010,
		ST011,
		ST018,
		Satellaview,
		Gameboy,
		GameboyColor,
		Sgb1GameboyCpu,
		Sgb2GameboyCpu,
		SGB1,
		SGB2,
		FDS,
		StudyBox
	}

	public struct MissingFirmwareMessage
	{
		public IntPtr Filename;
		public FirmwareType Firmware;
		public UInt32 Size;
		
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5)]
		public IntPtr[] FileHashes;

		public List<string> GetFileHashes()
		{
			List<string> hashes = new List<string>();
			for(int i = 0; i < FileHashes.Length; i++) {
				string hash = Marshal.PtrToStringUTF8(FileHashes[i]) ?? "";
				if(hash.Length > 0) {
					hashes.Add(hash);
				}
			}
			return hashes;
		}
	}

	public struct ExecuteShortcutParams
	{
		public EmulatorShortcut Shortcut;
		public UInt32 Param;
	};

	public enum AudioPlayerAction
	{
		NextTrack,
		PrevTrack,
		SelectTrack,
	}

	public struct AudioPlayerActionParams
	{
		public AudioPlayerAction Action;
		public UInt32 TrackNumber;
	}

	public enum TapeRecorderAction
	{
		Play,
		StartRecord,
		StopRecord
	}
}
