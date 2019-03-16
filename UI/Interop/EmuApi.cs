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
			return new Version((int)major, (int)minor, (int)revision, 0);
		}

		[DllImport(DllPath)] public static extern IntPtr RegisterNotificationCallback(NotificationListener.NotificationCallback callback);
		[DllImport(DllPath)] public static extern void UnregisterNotificationCallback(IntPtr notificationListener);

		[DllImport(DllPath)] public static extern void InitializeEmu([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string homeFolder, IntPtr windowHandle, IntPtr dxViewerHandle, [MarshalAs(UnmanagedType.I1)]bool noAudio, [MarshalAs(UnmanagedType.I1)]bool noVideo, [MarshalAs(UnmanagedType.I1)]bool noInput);
		[DllImport(DllPath)] public static extern void Release();

		[DllImport(DllPath)] public static extern void Run();
		[DllImport(DllPath)] public static extern void Stop();

		[DllImport(DllPath)] public static extern void Reset();
		[DllImport(DllPath)] public static extern void PowerCycle();

		[DllImport(DllPath)] public static extern void Pause();
		[DllImport(DllPath)] public static extern void Resume();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool IsPaused();

		[DllImport(DllPath)] public static extern void TakeScreenshot();

		[DllImport(DllPath)] public static extern void LoadRom(
			[MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string filepath,
			[MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string patchFile = ""
		);

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
		[DllImport(DllPath)] public static extern Int64 GetStateInfo(UInt32 stateIndex);
	}

	public struct ScreenSize
	{
		public Int32 Width;
		public Int32 Height;
		public double Scale;
	}
}
