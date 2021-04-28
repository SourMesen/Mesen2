using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	public class SnesConfig : BaseConfig<SnesConfig>
	{
		//Input
		[Reactive] public List<ControllerConfig> Controllers { get; set; } = new List<ControllerConfig> { new ControllerConfig(), new ControllerConfig(), new ControllerConfig(), new ControllerConfig(), new ControllerConfig() };

		[Reactive] public ConsoleRegion Region { get; set; } = ConsoleRegion.Auto;

		//Video
		[Reactive] public bool BlendHighResolutionModes { get; set; } = false;
		[Reactive] public bool HideBgLayer0 { get; set; } = false;
		[Reactive] public bool HideBgLayer1 { get; set; } = false;
		[Reactive] public bool HideBgLayer2 { get; set; } = false;
		[Reactive] public bool HideBgLayer3 { get; set; } = false;
		[Reactive] public bool HideSprites { get; set; } = false;
		[Reactive] public bool DisableFrameSkipping { get; set; } = false;

		[Reactive] [MinMax(0, 100)] public UInt32 OverscanLeft { get; set; } = 0;
		[Reactive] [MinMax(0, 100)] public UInt32 OverscanRight { get; set; } = 0;
		[Reactive] [MinMax(0, 100)] public UInt32 OverscanTop { get; set; } = 7;
		[Reactive] [MinMax(0, 100)] public UInt32 OverscanBottom { get; set; } = 8;

		//Audio
		[Reactive] public bool EnableCubicInterpolation { get; set; } = false;

		//Emulation
		[Reactive] public bool EnableRandomPowerOnState { get; set; } = false;
		[Reactive] public bool EnableStrictBoardMappings { get; set; } = false;
		[Reactive] public RamState RamPowerOnState { get; set; } = RamState.Random;

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
			while(Controllers.Count < 5) {
				Controllers.Add(new ControllerConfig());
			}

			//Force SNES controllers for multitap
			Controllers[2].Type = ControllerType.SnesController;
			Controllers[3].Type = ControllerType.SnesController;
			Controllers[4].Type = ControllerType.SnesController;

			ConfigApi.SetSnesConfig(new InteropSnesConfig() {
				Controllers = new InteropControllerConfig[5] {
					this.Controllers[0].ToInterop(),
					this.Controllers[1].ToInterop(),
					this.Controllers[2].ToInterop(),
					this.Controllers[3].ToInterop(),
					this.Controllers[4].ToInterop()
				},

				Region = this.Region,

				BlendHighResolutionModes = this.BlendHighResolutionModes,
				HideBgLayer0 = this.HideBgLayer0,
				HideBgLayer1 = this.HideBgLayer1,
				HideBgLayer2 = this.HideBgLayer2,
				HideBgLayer3 = this.HideBgLayer3,
				HideSprites = this.HideSprites,
				DisableFrameSkipping = this.DisableFrameSkipping,

				OverscanLeft = this.OverscanLeft,
				OverscanRight = this.OverscanRight,
				OverscanTop = this.OverscanTop,
				OverscanBottom = this.OverscanBottom,

				EnableCubicInterpolation = this.EnableCubicInterpolation,

				EnableRandomPowerOnState = this.EnableRandomPowerOnState,
				EnableStrictBoardMappings = this.EnableStrictBoardMappings,
				PpuExtraScanlinesBeforeNmi = this.PpuExtraScanlinesBeforeNmi,
				PpuExtraScanlinesAfterNmi = this.PpuExtraScanlinesAfterNmi,
				GsuClockSpeed = this.GsuClockSpeed,
				RamPowerOnState = this.RamPowerOnState,
				BsxCustomDate = this.BsxCustomDate.Ticks + this.BsxCustomTime.Ticks
			});
		}

		public void InitializeDefaults(DefaultKeyMappingType defaultMappings)
		{
			KeyPresets presets = new KeyPresets();
			List<KeyMapping> mappings = new List<KeyMapping>();
			if(defaultMappings.HasFlag(DefaultKeyMappingType.Xbox)) {
				mappings.Add(presets.XboxLayout1);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.Ps4)) {
				mappings.Add(presets.Ps4Layout1);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.WasdKeys)) {
				mappings.Add(presets.WasdLayout);
			}
			if(defaultMappings.HasFlag(DefaultKeyMappingType.ArrowKeys)) {
				mappings.Add(presets.ArrowLayout);
			}

			Controllers[0].Type = ControllerType.SnesController;
			Controllers[0].TurboSpeed = 2;
			if(mappings.Count > 0) {
				Controllers[0].Mapping1 = mappings[0];
				if(mappings.Count > 1) {
					Controllers[0].Mapping2 = mappings[1];
					if(mappings.Count > 2) {
						Controllers[0].Mapping3 = mappings[2];
						if(mappings.Count > 3) {
							Controllers[0].Mapping4 = mappings[3];
						}
					}
				}
			}
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropSnesConfig
	{
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5)]
		public InteropControllerConfig[] Controllers;

		public ConsoleRegion Region;

		[MarshalAs(UnmanagedType.I1)] public bool BlendHighResolutionModes;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer0;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer1;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer2;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer3;
		[MarshalAs(UnmanagedType.I1)] public bool HideSprites;
		[MarshalAs(UnmanagedType.I1)] public bool DisableFrameSkipping;

		public UInt32 OverscanLeft;
		public UInt32 OverscanRight;
		public UInt32 OverscanTop;
		public UInt32 OverscanBottom;

		[MarshalAs(UnmanagedType.I1)] public bool EnableCubicInterpolation;

		[MarshalAs(UnmanagedType.I1)] public bool EnableRandomPowerOnState;
		[MarshalAs(UnmanagedType.I1)] public bool EnableStrictBoardMappings;
		public RamState RamPowerOnState;

		public UInt32 PpuExtraScanlinesBeforeNmi;
		public UInt32 PpuExtraScanlinesAfterNmi;
		public UInt32 GsuClockSpeed;

		public long BsxCustomDate;
	}
}
