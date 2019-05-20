using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	[StructLayout(LayoutKind.Sequential)]
	public class AudioConfig : BaseConfig<AudioConfig>
	{
		[MarshalAs(UnmanagedType.LPStr)] public string AudioDevice = "";
		[MarshalAs(UnmanagedType.I1)] public bool EnableAudio = true;
		[MarshalAs(UnmanagedType.I1)] public bool DisableDynamicSampleRate = false;

		[MinMax(0, 100)] public UInt32 MasterVolume = 100;
		[ValidValues(11025, 22050, 32000, 44100, 48000, 96000)] public UInt32 SampleRate = 48000;
		[MinMax(15, 300)] public UInt32 AudioLatency = 60;

		[MarshalAs(UnmanagedType.I1)] public bool MuteSoundInBackground = false;
		[MarshalAs(UnmanagedType.I1)] public bool ReduceSoundInBackground = true;
		[MarshalAs(UnmanagedType.I1)] public bool ReduceSoundInFastForward = false;
		[MinMax(0, 100)] public int VolumeReduction = 75;

		[MarshalAs(UnmanagedType.I1)] public bool EnableEqualizer = false;
		[MinMax(-20.0, 20.0)] public double Band1Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band2Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band3Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band4Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band5Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band6Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band7Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band8Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band9Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band10Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band11Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band12Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band13Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band14Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band15Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band16Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band17Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band18Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band19Gain = 0;
		[MinMax(-20.0, 20.0)] public double Band20Gain = 0;

		public void ApplyConfig()
		{
			ConfigApi.SetAudioConfig(this);
		}
	}
}
