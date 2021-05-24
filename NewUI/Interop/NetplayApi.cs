using Mesen.Config;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Mesen.Interop
{
	public class NetplayApi
	{
		private const string DllPath = "MesenSCore.dll";

		[DllImport(DllPath)] public static extern void StartServer(UInt16 port, [MarshalAs(UnmanagedType.LPUTF8Str)]string password);
		[DllImport(DllPath)] public static extern void StopServer();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool IsServerRunning();
		[DllImport(DllPath)] public static extern void Connect([MarshalAs(UnmanagedType.LPUTF8Str)]string host, UInt16 port, [MarshalAs(UnmanagedType.LPUTF8Str)]string password, [MarshalAs(UnmanagedType.I1)]bool spectator);
		[DllImport(DllPath)] public static extern void Disconnect();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool IsConnected();

		[DllImport(DllPath)] public static extern Int32 NetPlayGetAvailableControllers();
		[DllImport(DllPath)] public static extern void NetPlaySelectController(Int32 controllerPort);
		[DllImport(DllPath)] public static extern Int32 NetPlayGetControllerPort();
	}
}
