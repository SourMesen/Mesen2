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
	public class ConfigApi
	{
		private const string DllPath = "MesenSCore.dll";

		[DllImport(DllPath)] public static extern void SetVideoConfig(VideoConfig config);
		[DllImport(DllPath)] public static extern void SetAudioConfig(AudioConfig config);

		[DllImport(DllPath, EntryPoint = "GetAudioDevices")] private static extern IntPtr GetAudioDevicesWrapper();
		public static List<string> GetAudioDevices()
		{
			return new List<string>(Utf8Marshaler.PtrToStringUtf8(ConfigApi.GetAudioDevicesWrapper()).Split(new string[1] { "||" }, StringSplitOptions.RemoveEmptyEntries));
		}
	}
}
