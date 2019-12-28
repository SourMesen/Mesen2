using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	[StructLayout(LayoutKind.Sequential)]
	public class VideoConfig : BaseConfig<VideoConfig>
	{
		[MinMax(0.1, 10.0)] public double VideoScale = 2;
		[MinMax(0.1, 5.0)] public double CustomAspectRatio = 1.0;
		public VideoFilterType VideoFilter = VideoFilterType.None;
		public VideoAspectRatio AspectRatio = VideoAspectRatio.NoStretching;

		[MarshalAs(UnmanagedType.I1)] public bool UseBilinearInterpolation = false;
		[MarshalAs(UnmanagedType.I1)] public bool BlendHighResolutionModes = false;
		[MarshalAs(UnmanagedType.I1)] public bool VerticalSync = false;
		[MarshalAs(UnmanagedType.I1)] public bool IntegerFpsMode = false;

		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer0 = false;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer1 = false;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer2 = false;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer3 = false;
		[MarshalAs(UnmanagedType.I1)] public bool HideSprites = false;
		[MarshalAs(UnmanagedType.I1)] public bool DisableFrameSkipping = false;

		[MinMax(-1, 1.0)] public double Brightness = 0;
		[MinMax(-1, 1.0)] public double Contrast = 0;
		[MinMax(-1, 1.0)] public double Hue = 0;
		[MinMax(-1, 1.0)] public double Saturation = 0;
		[MinMax(0, 1.0)] public double ScanlineIntensity = 0;

		[MinMax(-1, 1.0)] public double NtscArtifacts = 0;
		[MinMax(-1, 1.0)] public double NtscBleed = 0;
		[MinMax(-1, 1.0)] public double NtscFringing = 0;
		[MinMax(-1, 1.0)] public double NtscGamma = 0;
		[MinMax(-1, 1.0)] public double NtscResolution = 0;
		[MinMax(-1, 1.0)] public double NtscSharpness = 0;
		[MarshalAs(UnmanagedType.I1)] public bool NtscMergeFields = false;

		[MinMax(0, 100)] public UInt32 OverscanLeft = 0;
		[MinMax(0, 100)] public UInt32 OverscanRight = 0;
		[MinMax(0, 100)] public UInt32 OverscanTop = 0;
		[MinMax(0, 100)] public UInt32 OverscanBottom = 0;

		[MarshalAs(UnmanagedType.I1)] public bool FullscreenForceIntegerScale = false;
		[MarshalAs(UnmanagedType.I1)] public bool UseExclusiveFullscreen = false;
		public UInt32 ExclusiveFullscreenRefreshRate = 60;
		public UInt32 FullscreenResWidth = 0;
		public UInt32 FullscreenResHeight = 0;

		public void ApplyConfig()
		{
			ConfigApi.SetVideoConfig(this);
		}
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
