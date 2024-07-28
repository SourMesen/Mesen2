using Mesen.Interop;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class VideoConfig : BaseConfig<VideoConfig>
	{
		[Reactive] [MinMax(0.1, 5.0)] public double CustomAspectRatio { get; set; } = 1.0;
		[Reactive] public VideoFilterType VideoFilter { get; set; } = VideoFilterType.None;
		[Reactive] public VideoAspectRatio AspectRatio { get; set; } = VideoAspectRatio.NoStretching;

		[Reactive] public bool UseBilinearInterpolation { get; set; } = false;
		[Reactive] public bool UseSoftwareRenderer { get; set; } = false;
		[Reactive] public bool UseSrgbTextureFormat { get; set; } = false;
		[Reactive] public bool VerticalSync { get; set; } = false;
		[Reactive] public bool IntegerFpsMode { get; set; } = false;

		[Reactive] [MinMax(-100, 100)] public int Brightness { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int Contrast { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int Hue { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int Saturation { get; set; } = 0;
		[Reactive] [MinMax(0, 100)] public int ScanlineIntensity { get; set; } = 0;

		[Reactive][MinMax(0, 100)] public int LcdGridTopLeftBrightness { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public int LcdGridTopRightBrightness { get; set; } = 85;
		[Reactive][MinMax(0, 100)] public int LcdGridBottomLeftBrightness { get; set; } = 85;
		[Reactive][MinMax(0, 100)] public int LcdGridBottomRightBrightness { get; set; } = 85;

		[Reactive] [MinMax(-100, 100)] public int NtscArtifacts { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int NtscBleed { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int NtscFringing { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int NtscGamma { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int NtscResolution { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public int NtscSharpness { get; set; } = 0;
		[Reactive] public bool NtscMergeFields { get; set; } = false;

		[Reactive] public NtscBisqwitFilterScale NtscScale { get; set; } = NtscBisqwitFilterScale._2x;
		[Reactive] [MinMax(-50, 400)] public Int32 NtscYFilterLength { get; set; } = 0;
		[Reactive] [MinMax(0, 400)] public Int32 NtscIFilterLength { get; set; } = 50;
		[Reactive] [MinMax(0, 400)] public Int32 NtscQFilterLength { get; set; } = 50;

		[Reactive] public bool FullscreenForceIntegerScale { get; set; } = false;
		[Reactive] public bool UseExclusiveFullscreen { get; set; } = false;
		[Reactive] public UInt32 ExclusiveFullscreenRefreshRateNtsc { get; set; } = 60;
		[Reactive] public UInt32 ExclusiveFullscreenRefreshRatePal { get; set; } = 50;
		[Reactive] public FullscreenResolution ExclusiveFullscreenResolution { get; set; } = 0;

		[Reactive] public ScreenRotation ScreenRotation { get; set; } = ScreenRotation.None;

		public VideoConfig()
		{
		}

		public void ApplyConfig()
		{
			double customAspectRatio = CustomAspectRatio;
			VideoAspectRatio aspectRatio = AspectRatio;
			VideoFilterType videoFilter = VideoFilter;

			ConsoleOverrideConfig? overrides = ConsoleOverrideConfig.GetActiveOverride();
			if(overrides?.OverrideVideoFilter == true) {
				videoFilter = overrides.VideoFilter;
			}

			if(overrides?.OverrideAspectRatio == true) {
				aspectRatio = overrides.AspectRatio;
				customAspectRatio = overrides.CustomAspectRatio;
			}

			ConfigApi.SetVideoConfig(new InteropVideoConfig() {
				CustomAspectRatio = customAspectRatio,
				VideoFilter = videoFilter,
				AspectRatio = aspectRatio,

				UseBilinearInterpolation = this.UseBilinearInterpolation,
				UseSrgbTextureFormat = this.UseSrgbTextureFormat,
				VerticalSync = this.VerticalSync,
				IntegerFpsMode = this.IntegerFpsMode,

				Brightness = this.Brightness / 100.0,
				Contrast = this.Contrast / 100.0,
				Hue = this.Hue / 100.0,
				Saturation = this.Saturation / 100.0,
				ScanlineIntensity = this.ScanlineIntensity / 100.0,

				LcdGridTopLeftBrightness = this.LcdGridTopLeftBrightness / 100.0,
				LcdGridTopRightBrightness = this.LcdGridTopRightBrightness / 100.0,
				LcdGridBottomLeftBrightness = this.LcdGridBottomLeftBrightness / 100.0,
				LcdGridBottomRightBrightness = this.LcdGridBottomRightBrightness / 100.0,

				NtscArtifacts = this.NtscArtifacts / 100.0,
				NtscBleed = this.NtscBleed / 100.0,
				NtscFringing = this.NtscFringing / 100.0,
				NtscGamma = this.NtscGamma / 100.0,
				NtscResolution = this.NtscResolution / 100.0,
				NtscSharpness = this.NtscSharpness / 100.0,
				NtscMergeFields = this.NtscMergeFields,

				NtscScale = this.NtscScale,
				NtscYFilterLength = this.NtscYFilterLength / 100.0,
				NtscIFilterLength = this.NtscIFilterLength / 100.0,
				NtscQFilterLength = this.NtscQFilterLength / 100.0,

				FullscreenForceIntegerScale = this.FullscreenForceIntegerScale,
				UseExclusiveFullscreen = this.UseExclusiveFullscreen,
				ExclusiveFullscreenRefreshRateNtsc = this.ExclusiveFullscreenRefreshRateNtsc,
				ExclusiveFullscreenRefreshRatePal = this.ExclusiveFullscreenRefreshRatePal,
				FullscreenResWidth = (uint)(ExclusiveFullscreenResolution == FullscreenResolution.Default ? (ApplicationHelper.GetMainWindow()?.Screens.Primary?.Bounds.Width ?? 1920) : ExclusiveFullscreenResolution.GetWidth()),
				FullscreenResHeight = (uint)(ExclusiveFullscreenResolution == FullscreenResolution.Default ? (ApplicationHelper.GetMainWindow()?.Screens.Primary?.Bounds.Height ?? 1080) : ExclusiveFullscreenResolution.GetHeight()),

				ScreenRotation = (uint)ScreenRotation
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
		[MarshalAs(UnmanagedType.I1)] public bool UseSrgbTextureFormat;
		[MarshalAs(UnmanagedType.I1)] public bool VerticalSync;
		[MarshalAs(UnmanagedType.I1)] public bool IntegerFpsMode;

		public double Brightness;
		public double Contrast;
		public double Hue;
		public double Saturation;
		public double ScanlineIntensity;

		public double LcdGridTopLeftBrightness;
		public double LcdGridTopRightBrightness;
		public double LcdGridBottomLeftBrightness;
		public double LcdGridBottomRightBrightness;

		public double NtscArtifacts;
		public double NtscBleed;
		public double NtscFringing;
		public double NtscGamma;
		public double NtscResolution;
		public double NtscSharpness;
		[MarshalAs(UnmanagedType.I1)] public bool NtscMergeFields;

		public NtscBisqwitFilterScale NtscScale;
		public double NtscYFilterLength;
		public double NtscIFilterLength;
		public double NtscQFilterLength;

		[MarshalAs(UnmanagedType.I1)] public bool FullscreenForceIntegerScale;
		[MarshalAs(UnmanagedType.I1)] public bool UseExclusiveFullscreen;
		public UInt32 ExclusiveFullscreenRefreshRateNtsc;
		public UInt32 ExclusiveFullscreenRefreshRatePal;
		public UInt32 FullscreenResWidth;
		public UInt32 FullscreenResHeight;

		public UInt32 ScreenRotation;
	}

	public enum VideoFilterType
	{
		None = 0,
		NtscBlargg,
		NtscBisqwit,
		LcdGrid,
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

	public enum ScreenRotation
	{
		None = 0,
		_90Degrees = 90,
		_180Degrees = 180,
		_270Degrees = 270
	}

	public enum NtscBisqwitFilterScale
	{
		_2x,
		_4x,
		_8x
	}

	public enum FullscreenResolution
	{
		Default,
		_3840x2160,
		_2560x1440,
		_2160x1200,
		_1920x1440,
		_1920x1200,
		_1920x1080,
		_1680x1050,
		_1600x1200,
		_1600x1024,
		_1600x900,
		_1366x768,
		_1360x768,
		_1280x1024,
		_1280x960,
		_1280x800,
		_1280x768,
		_1280x720,
		_1152x864,
		_1024x768,
		_800x600,
		_640x480
	}

	public static class FullscreenResolutionExtensions
	{
		public static int GetWidth(this FullscreenResolution res)
		{
			return Int32.Parse(res.ToString().Substring(1, res.ToString().IndexOf("x") - 1));
		}

		public static int GetHeight(this FullscreenResolution res)
		{
			return Int32.Parse(res.ToString().Substring(res.ToString().IndexOf("x") + 1));
		}
	}
}
