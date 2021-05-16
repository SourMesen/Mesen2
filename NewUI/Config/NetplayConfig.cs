using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	[StructLayout(LayoutKind.Sequential)]
	public class NetplayConfig
	{
		public string Host = "localhost";
		public UInt16 Port = 8888;
		public string Password = "";

		public string PlayerName = "PlayerName";

		public string ServerName = "Default";
		public UInt16 ServerPort = 8888;
		public string ServerPassword = "";
	}
}
