using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	[StructLayout(LayoutKind.Sequential)]
	public class EmulationConfig : BaseConfig<EmulationConfig>
	{
		[MinMax(0, 5000)] public UInt32 EmulationSpeed = 100;
		[MinMax(0, 5000)] public UInt32 TurboSpeed = 300;
		[MinMax(0, 5000)] public UInt32 RewindSpeed = 100;

		public ConsoleRegion Region = ConsoleRegion.Auto;
		
		[MinMax(0, 10)] public UInt32 RunAheadFrames = 0;

		[MarshalAs(UnmanagedType.I1)] public bool EnableRandomPowerOnState = false;
		[MarshalAs(UnmanagedType.I1)] public bool EnableStrictBoardMappings = false;

		[MinMax(0, 1000)] public UInt32 PpuExtraScanlinesBeforeNmi = 0;
		[MinMax(0, 1000)] public UInt32 PpuExtraScanlinesAfterNmi = 0;
		[MinMax(100, 1000)] public UInt32 GsuClockSpeed = 100;

		public RamState RamPowerOnState = RamState.Random;

		public long BsxCustomDate = -1;
		
		public void ApplyConfig()
		{
			ConfigApi.SetEmulationConfig(this);
		}
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
