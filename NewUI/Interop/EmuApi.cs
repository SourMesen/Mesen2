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
			[MarshalAs(UnmanagedType.LPUTF8Str)]string patchFile = ""
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

		[DllImport(DllPath)] public static extern double GetAspectRatio();
		[DllImport(DllPath)] public static extern void SetRendererSize(UInt32 width, UInt32 height);

		[DllImport(DllPath)] public static extern void ExecuteShortcut(ExecuteShortcutParams p);

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
		//TODO
		/*public static Image GetSaveStatePreview(string saveStatePath)
		{
			if(File.Exists(saveStatePath)) {
				byte[] buffer = new byte[512*478*4];
				Int32 size = EmuApi.GetSaveStatePreviewWrapper(saveStatePath, buffer);
				if(size > 0) {
					Array.Resize(ref buffer, size);
					using(MemoryStream stream = new MemoryStream(buffer)) {
						return Image.FromStream(stream);
					}
				}
			}
			return null;
		}*/

		[DllImport(DllPath)] public static extern void SetCheats([In]UInt32[] cheats, UInt32 cheatCount);
		[DllImport(DllPath)] public static extern void ClearCheats();
	}

	public struct ScreenSize
	{
		public Int32 Width;
		public Int32 Height;
		public double Scale;
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
		Nsf,
		StudyBox
	}

	public struct InteropRomInfo
	{
		public IntPtr RomPath;
		public IntPtr PatchPath;
		public RomFormat Format;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 40)]
		public byte[] Sha1;
	}

	public class RomInfo
	{
		public string RomPath = "";
		public string PatchPath = "";
		public RomFormat Format = RomFormat.Unknown;
		public string Sha1 = "";

		public RomInfo() { }

		public RomInfo(InteropRomInfo romInfo)
		{
			RomPath = (ResourcePath)Utf8Utilities.GetStringFromIntPtr(romInfo.RomPath);
			PatchPath = (ResourcePath)Utf8Utilities.GetStringFromIntPtr(romInfo.PatchPath);
			Format = romInfo.Format;
			Sha1 = Encoding.UTF8.GetString(romInfo.Sha1);
		}

		public string GetRomName()
		{
			return Path.GetFileNameWithoutExtension(((ResourcePath)RomPath).FileName);
		}
	}

	public struct SnesCartInformation
	{
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
		public byte[] MakerCode;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 4)]
		public byte[] GameCode;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 7)]
		public byte[] Reserved;

		public byte ExpansionRamSize;
		public byte SpecialVersion;
		public byte CartridgeType;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 21)]
		public byte[] CartName;

		public byte MapMode;
		public byte RomType;
		public byte RomSize;
		public byte SramSize;

		public byte DestinationCode;
		public byte Reserved2;
		public byte Version;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
		public byte[] ChecksumComplement;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2)]
		public byte[] Checksum;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 0x20)]
		public byte[] CpuVectors;
	}

	public enum CoprocessorType
	{
		None,
		DSP1,
		DSP1B,
		DSP2,
		DSP3,
		DSP4,
		GSU,
		OBC1,
		SA1,
		SDD1,
		RTC,
		Satellaview,
		SPC7110,
		ST010,
		ST011,
		ST018,
		CX4,
		SGB
	}

	/*public static class CoprocessorTypeExtensions
	{
		public static CpuType? ToCpuType(this CoprocessorType type)
		{
			switch(type) {
				case CoprocessorType.CX4: 
					return CpuType.Cx4;

				case CoprocessorType.DSP1: case CoprocessorType.DSP1B: case CoprocessorType.DSP2: case CoprocessorType.DSP3: case CoprocessorType.DSP4: 
					return CpuType.NecDsp;

				case CoprocessorType.SA1:
					return CpuType.Sa1;

				case CoprocessorType.GSU:
					return CpuType.Gsu;

				case CoprocessorType.Gameboy: 
				case CoprocessorType.SGB: 
					return CpuType.Gameboy;

				default:
					return null;
			}
		}
	}*/

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
	}

	public struct MissingFirmwareMessage
	{
		public IntPtr Filename;
		public FirmwareType Firmware;
		public UInt32 Size;
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
}
