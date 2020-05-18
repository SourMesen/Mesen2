using System;
using System.Collections.Generic;
using System.Drawing;
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

		[DllImport(DllPath)] public static extern void InitializeEmu([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string homeFolder, IntPtr windowHandle, IntPtr dxViewerHandle, [MarshalAs(UnmanagedType.I1)]bool noAudio, [MarshalAs(UnmanagedType.I1)]bool noVideo, [MarshalAs(UnmanagedType.I1)]bool noInput);
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

		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool LoadRom(
			[MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string filepath,
			[MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string patchFile = ""
		);

		[DllImport(DllPath, EntryPoint = "GetRomInfo")] private static extern void GetRomInfoWrapper(out InteropRomInfo romInfo);
		public static RomInfo GetRomInfo()
		{
			InteropRomInfo info;
			EmuApi.GetRomInfoWrapper(out info);
			return new RomInfo(info);
		}

		[DllImport(DllPath)] public static extern void LoadRecentGame([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string filepath, [MarshalAs(UnmanagedType.I1)]bool resetGame);

		[DllImport(DllPath)] public static extern void AddKnownGameFolder([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string folder);

		[DllImport(DllPath)] public static extern void SetFolderOverrides(
			[MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string saveDataFolder,
			[MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string saveStateFolder,
			[MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string screenshotFolder
		);

		[DllImport(DllPath)] public static extern void SetDisplayLanguage(Language lang);
		[DllImport(DllPath)] public static extern void SetFullscreenMode([MarshalAs(UnmanagedType.I1)]bool fullscreen, IntPtr windowHandle, UInt32 monitorWidth, UInt32 monitorHeight);

		[DllImport(DllPath)] public static extern ScreenSize GetScreenSize([MarshalAs(UnmanagedType.I1)]bool ignoreScale);

		[DllImport(DllPath, EntryPoint = "GetLog")] private static extern IntPtr GetLogWrapper();
		public static string GetLog() { return Utf8Marshaler.PtrToStringUtf8(EmuApi.GetLogWrapper()).Replace("\n", Environment.NewLine); }
		[DllImport(DllPath)] public static extern void WriteLogEntry([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string message);
		[DllImport(DllPath)] public static extern void DisplayMessage([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string title, [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string message, [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string param1 = null);

		[DllImport(DllPath)] public static extern IntPtr GetArchiveRomList([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string filename);

		[DllImport(DllPath)] public static extern void SaveState(UInt32 stateIndex);
		[DllImport(DllPath)] public static extern void LoadState(UInt32 stateIndex);
		[DllImport(DllPath)] public static extern void SaveStateFile([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string filepath);
		[DllImport(DllPath)] public static extern void LoadStateFile([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string filepath);

		[DllImport(DllPath, EntryPoint = "GetSaveStatePreview")] private static extern Int32 GetSaveStatePreviewWrapper([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string saveStatePath, [Out]byte[] imgData);
		public static Image GetSaveStatePreview(string saveStatePath)
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
		}

		[DllImport(DllPath)] public static extern void SetCheats([In]UInt32[] cheats, UInt32 cheatCount);
		[DllImport(DllPath)] public static extern void ClearCheats();
	}

	public struct ScreenSize
	{
		public Int32 Width;
		public Int32 Height;
		public double Scale;
	}

	public struct InteropRomInfo
	{
		public IntPtr RomPath;
		public IntPtr PatchPath;
		public CoprocessorType CoprocessorType;
		public SnesCartInformation Header;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 40)]
		public byte[] Sha1;
	}

	public struct RomInfo
	{
		public string RomPath;
		public string PatchPath;
		public CoprocessorType CoprocessorType;
		public SnesCartInformation Header;
		public string Sha1;

		public RomInfo(InteropRomInfo romInfo)
		{
			RomPath = (ResourcePath)Utf8Marshaler.GetStringFromIntPtr(romInfo.RomPath);
			PatchPath = (ResourcePath)Utf8Marshaler.GetStringFromIntPtr(romInfo.PatchPath);
			Header = romInfo.Header;
			CoprocessorType = romInfo.CoprocessorType;
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
		Gameboy
	}

	public struct MissingFirmwareMessage
	{
		public IntPtr Filename;
		public CoprocessorType FirmwareType;
		public UInt32 Size;
	}
}
