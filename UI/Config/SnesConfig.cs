using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class SnesConfig : BaseConfig<SnesConfig>
	{
		[Reactive] public ConsoleOverrideConfig ConfigOverrides { get; set; } = new();

		//Input
		[Reactive] public SnesControllerConfig Port1 { get; set; } = new SnesControllerConfig();
		[Reactive] public SnesControllerConfig Port2 { get; set; } = new SnesControllerConfig();
		
		[Reactive] public SnesControllerConfig Port1A { get; set; } = new SnesControllerConfig();
		[Reactive] public SnesControllerConfig Port1B { get; set; } = new SnesControllerConfig();
		[Reactive] public SnesControllerConfig Port1C { get; set; } = new SnesControllerConfig();
		[Reactive] public SnesControllerConfig Port1D { get; set; } = new SnesControllerConfig();
		
		[Reactive] public SnesControllerConfig Port2A { get; set; } = new SnesControllerConfig();
		[Reactive] public SnesControllerConfig Port2B { get; set; } = new SnesControllerConfig();
		[Reactive] public SnesControllerConfig Port2C { get; set; } = new SnesControllerConfig();
		[Reactive] public SnesControllerConfig Port2D { get; set; } = new SnesControllerConfig();

		[Reactive] public bool AllowInvalidInput { get; set; } = false;

		[ValidValues(ConsoleRegion.Auto, ConsoleRegion.Ntsc, ConsoleRegion.Pal)]
		[Reactive] public ConsoleRegion Region { get; set; } = ConsoleRegion.Auto;

		//Video
		[Reactive] public bool BlendHighResolutionModes { get; set; } = false;
		[Reactive] public bool HideBgLayer1 { get; set; } = false;
		[Reactive] public bool HideBgLayer2 { get; set; } = false;
		[Reactive] public bool HideBgLayer3 { get; set; } = false;
		[Reactive] public bool HideBgLayer4 { get; set; } = false;
		[Reactive] public bool HideSprites { get; set; } = false;
		[Reactive] public bool DisableFrameSkipping { get; set; } = false;
		[Reactive] public bool ForceFixedResolution { get; set; } = false;

		[Reactive] public OverscanConfig Overscan { get; set; } = new() { Top = 7, Bottom = 8 };

		//Audio
		[Reactive] public DspInterpolationType InterpolationType { get; set; } = DspInterpolationType.Gauss;
		[Reactive][MinMax(0, 100)] public UInt32 Channel1Vol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 Channel2Vol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 Channel3Vol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 Channel4Vol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 Channel5Vol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 Channel6Vol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 Channel7Vol { get; set; } = 100;
		[Reactive][MinMax(0, 100)] public UInt32 Channel8Vol { get; set; } = 100;

		//Emulation
		[Reactive] public bool EnableRandomPowerOnState { get; set; } = false;
		[Reactive] public bool EnableStrictBoardMappings { get; set; } = false;
		[Reactive] public RamState RamPowerOnState { get; set; } = RamState.Random;
		[Reactive] [MinMax(-999, 999)] public Int32 SpcClockSpeedAdjustment { get; set; } = 40;

		//Overclocking
		[Reactive] [MinMax(0, 1000)] public UInt32 PpuExtraScanlinesBeforeNmi { get; set; } = 0;
		[Reactive] [MinMax(0, 1000)] public UInt32 PpuExtraScanlinesAfterNmi { get; set; } = 0;
		[Reactive] [MinMax(100, 1000)] public UInt32 GsuClockSpeed { get; set; } = 100;

		//BSX
		[Reactive] public bool BsxUseCustomTime { get; set; } = false;
		[Reactive] public DateTimeOffset BsxCustomDate { get; set; } = new DateTimeOffset(1995, 1, 1, 0, 0, 0, TimeSpan.Zero);
		[Reactive] public TimeSpan BsxCustomTime { get; set; } = TimeSpan.Zero;

		public void ApplyConfig()
		{
			ConfigManager.Config.Video.ApplyConfig();

			ConfigApi.SetSnesConfig(new InteropSnesConfig() {
				Port1 = Port1.ToInterop(),
				Port1A = Port1A.ToInterop(),
				Port1B = Port1B.ToInterop(),
				Port1C = Port1C.ToInterop(),
				Port1D = Port1D.ToInterop(),

				Port2 = Port2.ToInterop(),
				Port2A = Port2A.ToInterop(),
				Port2B = Port2B.ToInterop(),
				Port2C = Port2C.ToInterop(),
				Port2D = Port2D.ToInterop(),

				Region = this.Region,

				AllowInvalidInput = this.AllowInvalidInput,

				BlendHighResolutionModes = this.BlendHighResolutionModes,
				HideBgLayer1 = this.HideBgLayer1,
				HideBgLayer2 = this.HideBgLayer2,
				HideBgLayer3 = this.HideBgLayer3,
				HideBgLayer4 = this.HideBgLayer4,
				HideSprites = this.HideSprites,
				
				DisableFrameSkipping = DisableFrameSkipping,
				ForceFixedResolution = ForceFixedResolution,

				Overscan = Overscan.ToInterop(),

				InterpolationType = InterpolationType,

				Channel1Vol = Channel1Vol,
				Channel2Vol = Channel2Vol,
				Channel3Vol = Channel3Vol,
				Channel4Vol = Channel4Vol,
				Channel5Vol = Channel5Vol,
				Channel6Vol = Channel6Vol,
				Channel7Vol = Channel7Vol,
				Channel8Vol = Channel8Vol,

				EnableRandomPowerOnState = this.EnableRandomPowerOnState,
				EnableStrictBoardMappings = this.EnableStrictBoardMappings,
				PpuExtraScanlinesBeforeNmi = this.PpuExtraScanlinesBeforeNmi,
				PpuExtraScanlinesAfterNmi = this.PpuExtraScanlinesAfterNmi,
				GsuClockSpeed = this.GsuClockSpeed,
				RamPowerOnState = this.RamPowerOnState,
				SpcClockSpeedAdjustment = this.SpcClockSpeedAdjustment,
				BsxCustomDate = BsxUseCustomTime ? (this.BsxCustomDate.ToUnixTimeSeconds() + (long)this.BsxCustomTime.TotalSeconds) : -1
			});
		}

		public void InitializeDefaults(DefaultKeyMappingType defaultMappings)
		{
			Port1.InitDefaults<SnesKeyMapping>(defaultMappings, ControllerType.SnesController);
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropSnesConfig
	{
		public InteropControllerConfig Port1;
		public InteropControllerConfig Port2;
		
		public InteropControllerConfig Port1A;
		public InteropControllerConfig Port1B;
		public InteropControllerConfig Port1C;
		public InteropControllerConfig Port1D;
		
		public InteropControllerConfig Port2A;
		public InteropControllerConfig Port2B;
		public InteropControllerConfig Port2C;
		public InteropControllerConfig Port2D;

		public ConsoleRegion Region;

		[MarshalAs(UnmanagedType.I1)] public bool AllowInvalidInput;
		[MarshalAs(UnmanagedType.I1)] public bool BlendHighResolutionModes;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer1;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer2;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer3;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer4;
		[MarshalAs(UnmanagedType.I1)] public bool HideSprites;
		[MarshalAs(UnmanagedType.I1)] public bool DisableFrameSkipping;
		[MarshalAs(UnmanagedType.I1)] public bool ForceFixedResolution;

		public InteropOverscanDimensions Overscan;

		public DspInterpolationType InterpolationType;
		public UInt32 Channel1Vol;
		public UInt32 Channel2Vol;
		public UInt32 Channel3Vol;
		public UInt32 Channel4Vol;
		public UInt32 Channel5Vol;
		public UInt32 Channel6Vol;
		public UInt32 Channel7Vol;
		public UInt32 Channel8Vol;

		[MarshalAs(UnmanagedType.I1)] public bool EnableRandomPowerOnState;
		[MarshalAs(UnmanagedType.I1)] public bool EnableStrictBoardMappings;
		public RamState RamPowerOnState;
		public Int32 SpcClockSpeedAdjustment;

		public UInt32 PpuExtraScanlinesBeforeNmi;
		public UInt32 PpuExtraScanlinesAfterNmi;
		public UInt32 GsuClockSpeed;

		public long BsxCustomDate;
	}

	public enum DspInterpolationType
	{
		Gauss,
		Cubic,
		Sinc,
		None
	}
}