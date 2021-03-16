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
	public class VideoConfig : ReactiveObject
	{
		[Reactive] [MinMax(0.1, 10.0)] public double VideoScale { get; set; } = 2;
		[Reactive] [MinMax(0.1, 5.0)] public double CustomAspectRatio { get; set; } = 1.0;
		[Reactive] public VideoFilterType VideoFilter { get; set; } = VideoFilterType.None;
		[Reactive] public VideoAspectRatio AspectRatio { get; set; } = VideoAspectRatio.NoStretching;

		[Reactive] public bool UseBilinearInterpolation { get; set; } = false;
		[Reactive] public bool BlendHighResolutionModes { get; set; } = false;
		[Reactive] public bool VerticalSync { get; set; } = false;
		[Reactive] public bool IntegerFpsMode { get; set; } = false;

		[Reactive] public bool HideBgLayer0 { get; set; } = false;
		[Reactive] public bool HideBgLayer1 { get; set; } = false;
		[Reactive] public bool HideBgLayer2 { get; set; } = false;
		[Reactive] public bool HideBgLayer3 { get; set; } = false;
		[Reactive] public bool HideSprites { get; set; } = false;
		[Reactive] public bool DisableFrameSkipping { get; set; } = false;

		[Reactive] [MinMax(-1, 1.0)] public double Brightness { get; set; } = 0;
		[Reactive] [MinMax(-1, 1.0)] public double Contrast { get; set; } = 0;
		[Reactive] [MinMax(-1, 1.0)] public double Hue { get; set; } = 0;
		[Reactive] [MinMax(-1, 1.0)] public double Saturation { get; set; } = 0;
		[Reactive] [MinMax(0, 1.0)] public double ScanlineIntensity { get; set; } = 0;

		[Reactive] [MinMax(-1, 1.0)] public double NtscArtifacts { get; set; } = 0;
		[Reactive] [MinMax(-1, 1.0)] public double NtscBleed { get; set; } = 0;
		[Reactive] [MinMax(-1, 1.0)] public double NtscFringing { get; set; } = 0;
		[Reactive] [MinMax(-1, 1.0)] public double NtscGamma { get; set; } = 0;
		[Reactive] [MinMax(-1, 1.0)] public double NtscResolution { get; set; } = 0;
		[Reactive] [MinMax(-1, 1.0)] public double NtscSharpness { get; set; } = 0;
		[Reactive] public bool NtscMergeFields { get; set; } = false;

		[Reactive] [MinMax(0, 100)] public UInt32 OverscanLeft { get; set; } = 0;
		[Reactive] [MinMax(0, 100)] public UInt32 OverscanRight { get; set; } = 0;
		[Reactive] [MinMax(0, 100)] public UInt32 OverscanTop { get; set; } = 7;
		[Reactive] [MinMax(0, 100)] public UInt32 OverscanBottom { get; set; } = 8;

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
				VideoScale = this.VideoScale,
				CustomAspectRatio = this.CustomAspectRatio,
				VideoFilter = this.VideoFilter,
				AspectRatio = this.AspectRatio,

				UseBilinearInterpolation = this.UseBilinearInterpolation,
				BlendHighResolutionModes = this.BlendHighResolutionModes,
				VerticalSync = this.VerticalSync,
				IntegerFpsMode = this.IntegerFpsMode,

				HideBgLayer0 = this.HideBgLayer0,
				HideBgLayer1 = this.HideBgLayer1,
				HideBgLayer2 = this.HideBgLayer2,
				HideBgLayer3 = this.HideBgLayer3,
				HideSprites = this.HideSprites,
				DisableFrameSkipping = this.DisableFrameSkipping,

				Brightness = this.Brightness,
				Contrast = this.Contrast,
				Hue = this.Hue,
				Saturation = this.Saturation,
				ScanlineIntensity = this.ScanlineIntensity,

				NtscArtifacts = this.NtscArtifacts,
				NtscBleed = this.NtscBleed,
				NtscFringing = this.NtscFringing,
				NtscGamma = this.NtscGamma,
				NtscResolution = this.NtscResolution,
				NtscSharpness = this.NtscSharpness,
				NtscMergeFields = this.NtscMergeFields,

				OverscanLeft = this.OverscanLeft,
				OverscanRight = this.OverscanRight,
				OverscanTop = this.OverscanTop,
				OverscanBottom = this.OverscanBottom,

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
		public double VideoScale;
		public double CustomAspectRatio;
		public VideoFilterType VideoFilter;
		public VideoAspectRatio AspectRatio;

		[MarshalAs(UnmanagedType.I1)] public bool UseBilinearInterpolation;
		[MarshalAs(UnmanagedType.I1)] public bool BlendHighResolutionModes;
		[MarshalAs(UnmanagedType.I1)] public bool VerticalSync;
		[MarshalAs(UnmanagedType.I1)] public bool IntegerFpsMode;

		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer0;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer1;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer2;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer3;
		[MarshalAs(UnmanagedType.I1)] public bool HideSprites;
		[MarshalAs(UnmanagedType.I1)] public bool DisableFrameSkipping;

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

		public UInt32 OverscanLeft;
		public UInt32 OverscanRight;
		public UInt32 OverscanTop;
		public UInt32 OverscanBottom;

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
