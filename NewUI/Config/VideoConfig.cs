using Mesen.Interop;
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
	public class VideoConfig : BaseConfig<VideoConfig>
	{
		[Reactive] [MinMax(0.1, 5.0)] public double CustomAspectRatio { get; set; } = 1.0;
		[Reactive] public VideoFilterType VideoFilter { get; set; } = VideoFilterType.None;
		[Reactive] public VideoAspectRatio AspectRatio { get; set; } = VideoAspectRatio.NoStretching;

		[Reactive] public bool UseBilinearInterpolation { get; set; } = false;
		[Reactive] public bool VerticalSync { get; set; } = false;
		[Reactive] public bool IntegerFpsMode { get; set; } = false;

		[Reactive] [MinMax(-100, 100)] public int Brightness { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int Contrast { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int Hue { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int Saturation { get; set; } = 0;
		[Reactive] [MinMax(0, 100)] public int ScanlineIntensity { get; set; } = 0;

		[Reactive] [MinMax(-100, 100)] public int NtscArtifacts { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int NtscBleed { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int NtscFringing { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int NtscGamma { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int NtscResolution { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int NtscSharpness { get; set; } = 0;
		[Reactive] public bool NtscMergeFields { get; set; } = false;

		[Reactive] public bool FullscreenForceIntegerScale { get; set; } = false;
		[Reactive] public bool UseExclusiveFullscreen { get; set; } = false;
		[Reactive] public UInt32 ExclusiveFullscreenRefreshRate { get; set; } = 60;
		[Reactive] public UInt32 FullscreenResWidth { get; set; } = 0;
		[Reactive] public UInt32 FullscreenResHeight { get; set; } = 0;

		public VideoConfig()
		{
		}

		public void ApplyConfig()
		{
			ConfigApi.SetVideoConfig(new InteropVideoConfig() {
				CustomAspectRatio = this.CustomAspectRatio,
				VideoFilter = this.VideoFilter,
				AspectRatio = this.AspectRatio,

				UseBilinearInterpolation = this.UseBilinearInterpolation,
				VerticalSync = this.VerticalSync,
				IntegerFpsMode = this.IntegerFpsMode,

				Brightness = this.Brightness / 100.0,
				Contrast = this.Contrast / 100.0,
				Hue = this.Hue / 100.0,
				ScanlineIntensity = this.ScanlineIntensity / 100.0,

				NtscArtifacts = this.NtscArtifacts / 100.0,
				NtscBleed = this.NtscBleed / 100.0,
				NtscFringing = this.NtscFringing / 100.0,
				NtscGamma = this.NtscGamma / 100.0,
				NtscResolution = this.NtscResolution / 100.0,
				NtscSharpness = this.NtscSharpness / 100.0,
				NtscMergeFields = this.NtscMergeFields,

				FullscreenForceIntegerScale = this.FullscreenForceIntegerScale,
				UseExclusiveFullscreen = this.UseExclusiveFullscreen,
				ExclusiveFullscreenRefreshRate = this.ExclusiveFullscreenRefreshRate,
				FullscreenResWidth = this.FullscreenResWidth,
				FullscreenResHeight = this.FullscreenResHeight,
			});
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropVideoConfig
	{
		public double CustomAspectRatio;
		public VideoFilterType VideoFilter;
		public VideoAspectRatio AspectRatio;

		[MarshalAs(UnmanagedType.I1)] public bool UseBilinearInterpolation;
		[MarshalAs(UnmanagedType.I1)] public bool VerticalSync;
		[MarshalAs(UnmanagedType.I1)] public bool IntegerFpsMode;

		public double Brightness;
		public double Contrast;
		public double Hue;
		public double Saturation;
		public double ScanlineIntensity;

		public double NtscArtifacts;
		public double NtscBleed;
		public double NtscFringing;
		public double NtscGamma;
		public double NtscResolution;
		public double NtscSharpness;
		public bool NtscMergeFields;

		[MarshalAs(UnmanagedType.I1)] public bool FullscreenForceIntegerScale;
		[MarshalAs(UnmanagedType.I1)] public bool UseExclusiveFullscreen;
		public UInt32 ExclusiveFullscreenRefreshRate;
		public UInt32 FullscreenResWidth;
		public UInt32 FullscreenResHeight;
	}
	
	public enum VideoFilterType
	{
		None = 0,
		NTSC,
		xBRZ2x,
		xBRZ3x,
		xBRZ4x,
		xBRZ5x,
		xBRZ6x,
		HQ2x,
		HQ3x,
		HQ4x,
		Scale2x,
		Scale3x,
		Scale4x,
		_2xSai,
		Super2xSai,
		SuperEagle,
		Prescale2x,
		Prescale3x,
		Prescale4x,
		Prescale6x,
		Prescale8x,
		Prescale10x
	}

	public enum VideoAspectRatio
	{
		NoStretching = 0,
		Auto = 1,
		NTSC = 2,
		PAL = 3,
		Standard = 4,
		Widescreen = 5,
		Custom = 6
	}
}
