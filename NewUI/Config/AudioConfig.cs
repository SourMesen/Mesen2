using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	public class AudioConfig : BaseConfig<AudioConfig>
	{
		[Reactive] public string AudioDevice { get; set; } = "";
		[Reactive] public bool EnableAudio { get; set; } = true;
		[Reactive] public bool DisableDynamicSampleRate { get; set; } = false;

		[Reactive] [MinMax(0, 100)] public UInt32 MasterVolume { get; set; } = 100;
		[Reactive] [ValidValues(11025, 22050, 32000, 44100, 48000, 96000)] public UInt32 SampleRate { get; set; } = 48000;
		[Reactive] [MinMax(15, 300)] public UInt32 AudioLatency { get; set; } = 60;

		[Reactive] public bool EnableCubicInterpolation { get; set; } = false;

		[Reactive] public bool MuteSoundInBackground { get; set; } = false;
		[Reactive] public bool ReduceSoundInBackground { get; set; } = true;
		[Reactive] public bool ReduceSoundInFastForward { get; set; } = false;
		[Reactive] [MinMax(0, 100)] public int VolumeReduction { get; set; } = 75;

		[Reactive] public bool EnableEqualizer { get; set; } = false;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band1Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band2Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band3Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band4Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band5Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band6Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band7Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band8Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band9Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band10Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band11Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band12Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band13Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band14Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band15Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band16Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band17Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band18Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band19Gain { get; set; } = 0;
		[Reactive] [MinMax(-20.0, 20.0)] public double Band20Gain { get; set; } = 0;

		public void ApplyConfig()
		{
			ConfigApi.SetAudioConfig(new InteropAudioConfig() {
				AudioDevice = this.AudioDevice,
				EnableAudio = this.EnableAudio,
				DisableDynamicSampleRate = this.DisableDynamicSampleRate,

				MasterVolume = this.MasterVolume,
				SampleRate = this.SampleRate,
				AudioLatency = this.AudioLatency,

				EnableCubicInterpolation = this.EnableCubicInterpolation,

				MuteSoundInBackground = this.MuteSoundInBackground,
				ReduceSoundInBackground = this.ReduceSoundInBackground,
				ReduceSoundInFastForward = this.ReduceSoundInFastForward,
				VolumeReduction = this.VolumeReduction,

				EnableEqualizer = this.EnableEqualizer,
				Band1Gain = this.Band1Gain,
				Band2Gain = this.Band2Gain,
				Band3Gain = this.Band3Gain,
				Band4Gain = this.Band4Gain,
				Band5Gain = this.Band5Gain,
				Band6Gain = this.Band6Gain,
				Band7Gain = this.Band7Gain,
				Band8Gain = this.Band8Gain,
				Band9Gain = this.Band9Gain,
				Band10Gain = this.Band10Gain,
				Band11Gain = this.Band11Gain,
				Band12Gain = this.Band12Gain,
				Band13Gain = this.Band13Gain,
				Band14Gain = this.Band14Gain,
				Band15Gain = this.Band15Gain,
				Band16Gain = this.Band16Gain,
				Band17Gain = this.Band17Gain,
				Band18Gain = this.Band18Gain,
				Band19Gain = this.Band19Gain,
				Band20Gain = this.Band20Gain
			});
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropAudioConfig
	{
		[MarshalAs(UnmanagedType.LPStr)] public string AudioDevice;
		[MarshalAs(UnmanagedType.I1)] public bool EnableAudio;
		[MarshalAs(UnmanagedType.I1)] public bool DisableDynamicSampleRate;

		public UInt32 MasterVolume;
		public UInt32 SampleRate;
		public UInt32 AudioLatency;

		[MarshalAs(UnmanagedType.I1)] public bool EnableCubicInterpolation;

		[MarshalAs(UnmanagedType.I1)] public bool MuteSoundInBackground;
		[MarshalAs(UnmanagedType.I1)] public bool ReduceSoundInBackground;
		[MarshalAs(UnmanagedType.I1)] public bool ReduceSoundInFastForward;
		public int VolumeReduction;

		[MarshalAs(UnmanagedType.I1)] public bool EnableEqualizer;
		public double Band1Gain;
		public double Band2Gain;
		public double Band3Gain;
		public double Band4Gain;
		public double Band5Gain;
		public double Band6Gain;
		public double Band7Gain;
		public double Band8Gain;
		public double Band9Gain;
		public double Band10Gain;
		public double Band11Gain;
		public double Band12Gain;
		public double Band13Gain;
		public double Band14Gain;
		public double Band15Gain;
		public double Band16Gain;
		public double Band17Gain;
		public double Band18Gain;
		public double Band19Gain;
		public double Band20Gain;
	}
}