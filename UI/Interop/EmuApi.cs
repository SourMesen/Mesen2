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

		[DllImport(DllPath)] public static extern IntPtr RegisterNotificationCallback(NotificationListener.NotificationCallback callback);
		[DllImport(DllPath)] public static extern void UnregisterNotificationCallback(IntPtr notificationListener);

		[DllImport(DllPath)] public static extern void InitializeEmu([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string homeFolder, IntPtr windowHandle, IntPtr dxViewerHandle, [MarshalAs(UnmanagedType.I1)]bool noAudio, [MarshalAs(UnmanagedType.I1)]bool noVideo, [MarshalAs(UnmanagedType.I1)]bool noInput);
		[DllImport(DllPath)] public static extern void Release();

		[DllImport(DllPath)] public static extern void Run();
		[DllImport(DllPath)] public static extern void Stop();

		[DllImport(DllPath)] public static extern void LoadRom(
			[MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string filepath,
			[MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string patchFile = ""
		);

		[DllImport(DllPath)] public static extern void SetKeyState(Int32 scanCode, [MarshalAs(UnmanagedType.I1)]bool pressed);
		[DllImport(DllPath)] public static extern void ResetKeyState();
		[DllImport(DllPath)] public static extern void SetMousePosition(double x, double y);

		[DllImport(DllPath)] public static extern void SetDisplayLanguage(Language lang);
	}
}
