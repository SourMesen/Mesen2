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
	public class EmulationConfig : BaseConfig<EmulationConfig>
	{
		[Reactive] [MinMax(0, 5000)] public UInt32 EmulationSpeed { get; set; } = 100;
		[Reactive] [MinMax(0, 5000)] public UInt32 TurboSpeed { get; set; } = 300;
		[Reactive] [MinMax(0, 5000)] public UInt32 RewindSpeed { get; set; } = 100;

		[Reactive] public ConsoleRegion Region { get; set; } = ConsoleRegion.Auto;

		[Reactive] [MinMax(0, 10)] public UInt32 RunAheadFrames { get; set; } = 0;

		[Reactive] public bool EnableRandomPowerOnState { get; set; } = false;
		[Reactive] public bool EnableStrictBoardMappings { get; set; } = false;

		[Reactive] [MinMax(0, 1000)] public UInt32 PpuExtraScanlinesBeforeNmi { get; set; } = 0;
		[Reactive] [MinMax(0, 1000)] public UInt32 PpuExtraScanlinesAfterNmi { get; set; } = 0;
		[Reactive] [MinMax(100, 1000)] public UInt32 GsuClockSpeed { get; set; } = 100;

		[Reactive] public RamState RamPowerOnState { get; set; } = RamState.Random;

		[Reactive] public bool BsxUseCustomTime { get; set; } = false;
		[Reactive] public DateTimeOffset BsxCustomDate { get; set; } = new DateTimeOffset(1995, 1, 1, 0, 0, 0, TimeSpan.Zero);
		[Reactive] public TimeSpan BsxCustomTime { get; set; } = TimeSpan.Zero;
		
		public void ApplyConfig()
		{
			ConfigApi.SetEmulationConfig(new InteropEmulationConfig() {
				EmulationSpeed = this.EmulationSpeed,
				TurboSpeed = this.TurboSpeed,
				RewindSpeed = this.RewindSpeed,
				Region = this.Region,
				RunAheadFrames = this.RunAheadFrames,
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
	public struct InteropEmulationConfig
	{
		public UInt32 EmulationSpeed;
		public UInt32 TurboSpeed;
		public UInt32 RewindSpeed;

		public ConsoleRegion Region;

		public UInt32 RunAheadFrames;

		[MarshalAs(UnmanagedType.I1)] public bool EnableRandomPowerOnState;
		[MarshalAs(UnmanagedType.I1)] public bool EnableStrictBoardMappings;

		public UInt32 PpuExtraScanlinesBeforeNmi;
		public UInt32 PpuExtraScanlinesAfterNmi;
		public UInt32 GsuClockSpeed;

		public RamState RamPowerOnState;

		public long BsxCustomDate;
	}

	public enum ConsoleRegion
	{
		Auto = 0,
		Ntsc = 1,
		Pal = 2
	}

	public enum RamState
	{
		Random = 0,
		AllZeros = 1,
		AllOnes = 2,
	}
}
