using Mesen.GUI.Config;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI
{
	class RecordApi
	{
		private const string DllPath = "MesenSCore.dll";

		[DllImport(DllPath)] public static extern void AviRecord([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(Utf8Marshaler))]string filename, VideoCodec codec, UInt32 compressionLevel);
		[DllImport(DllPath)] public static extern void AviStop();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool AviIsRecording();

		/*[DllImport(DllPath)] public static extern void WaveRecord([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(UTF8Marshaler))]string filename);
		[DllImport(DllPath)] public static extern void WaveStop();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool WaveIsRecording();

		[DllImport(DllPath)] public static extern void MoviePlay([MarshalAs(UnmanagedType.CustomMarshaler, MarshalTypeRef = typeof(UTF8Marshaler))]string filename);
		[DllImport(DllPath)] public static extern void MovieRecord(ref RecordMovieOptions options);
		[DllImport(DllPath)] public static extern void MovieStop();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool MoviePlaying();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool MovieRecording();*/
	}
}
