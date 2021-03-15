using Mesen.GUI.Config;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Mesen.GUI
{
	public class NetplayApi
	{
		private const string DllPath = "MesenSCore.dll";

		[DllImport(DllPath)] public static extern void StartServer(UInt16 port, [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string password, [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string hostPlayerName);
		[DllImport(DllPath)] public static extern void StopServer();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool IsServerRunning();
		[DllImport(DllPath)] public static extern void Connect([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string host, UInt16 port, [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string password, [MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string playerName, [MarshalAs(UnmanagedType.I1)]bool spectator);
		[DllImport(DllPath)] public static extern void Disconnect();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool IsConnected();

		[DllImport(DllPath)] public static extern Int32 NetPlayGetAvailableControllers();
		[DllImport(DllPath)] public static extern void NetPlaySelectController(Int32 controllerPort);
		[DllImport(DllPath)] public static extern Int32 NetPlayGetControllerPort();
	}
}
