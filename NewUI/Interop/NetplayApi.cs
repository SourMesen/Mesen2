using Mesen.Config;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Mesen.Interop
{
	public class NetplayApi
	{
		private const string DllPath = EmuApi.DllName;

		[DllImport(DllPath)] public static extern void StartServer(UInt16 port, [MarshalAs(UnmanagedType.LPUTF8Str)]string password);
		[DllImport(DllPath)] public static extern void StopServer();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool IsServerRunning();
		[DllImport(DllPath)] public static extern void Connect([MarshalAs(UnmanagedType.LPUTF8Str)]string host, UInt16 port, [MarshalAs(UnmanagedType.LPUTF8Str)]string password, [MarshalAs(UnmanagedType.I1)]bool spectator);
		[DllImport(DllPath)] public static extern void Disconnect();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool IsConnected();

		[DllImport(DllPath)] private static extern void NetPlayGetControllerList([In,Out] NetplayControllerUsageInfo[] controllers, ref Int32 length);
		public static NetplayControllerUsageInfo[] NetPlayGetControllerList()
		{
			NetplayControllerUsageInfo[] controllers = new NetplayControllerUsageInfo[16];
			int length = 16;
			NetplayApi.NetPlayGetControllerList(controllers, ref length);
			Array.Resize(ref controllers, length);
			return controllers;
		}

		[DllImport(DllPath)] public static extern void NetPlaySelectController(NetplayControllerInfo controllerPort);
		[DllImport(DllPath)] public static extern NetplayControllerInfo NetPlayGetControllerPort();
	}

	public struct NetplayControllerInfo
	{
		public byte Port;
		public byte SubPort;
	}

	public struct NetplayControllerUsageInfo
	{
		public NetplayControllerInfo Port;
		public ControllerType Type;
		[MarshalAs(UnmanagedType.I1)] public bool InUse;
	}
}
