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
		//Video
		[Reactive] public bool BlendHighResolutionModes { get; set; } = false;
		[Reactive] public bool HideBgLayer0 { get; set; } = false;
		[Reactive] public bool HideBgLayer1 { get; set; } = false;
		[Reactive] public bool HideBgLayer2 { get; set; } = false;
		[Reactive] public bool HideBgLayer3 { get; set; } = false;
		[Reactive] public bool HideSprites { get; set; } = false;
		[Reactive] public bool DisableFrameSkipping { get; set; } = false;

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
			ConfigApi.SetSnesConfig(new InteropSnesConfig() {
				BlendHighResolutionModes = this.BlendHighResolutionModes,
				HideBgLayer0 = this.HideBgLayer0,
				HideBgLayer1 = this.HideBgLayer1,
				HideBgLayer2 = this.HideBgLayer2,
				HideBgLayer3 = this.HideBgLayer3,
				HideSprites = this.HideSprites,
				DisableFrameSkipping = this.DisableFrameSkipping,

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
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropSnesConfig
	{
		[MarshalAs(UnmanagedType.I1)] public bool BlendHighResolutionModes;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer0;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer1;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer2;
		[MarshalAs(UnmanagedType.I1)] public bool HideBgLayer3;
		[MarshalAs(UnmanagedType.I1)] public bool HideSprites;
		[MarshalAs(UnmanagedType.I1)] public bool DisableFrameSkipping;

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
